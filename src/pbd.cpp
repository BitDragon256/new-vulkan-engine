#include "pbd.h"

#include <numeric>

#include <linalg/gsl_linalg.h>

PBDParticle::PBDParticle() :
      position{ 0.f }, oldPosition{ 0.f }, velocity{ 0.f }, mass{ 1.f }, invmass{ 1.f }
{}

PBDSystem::PBDSystem() :
      m_grid{ m_gridSize }, m_collisionConstraintStart{0}
{}

void PBDSystem::awake(EntityId id)
{
      auto& particle = m_ecs->get_component<PBDParticle>(id);
      particle.oldPosition = particle.position;
      m_grid.insert_particle(particle.position, id);
}

void PBDSystem::start()
{
      
}
void PBDSystem::update(float dt)
{
      for (EntityId entity : m_entities)
      {
            auto& particle = get_particle(entity);
            if (particle.invmass != 0)
                  particle.invmass = 1.f / particle.mass;
            particle.velocity += dt * particle.invmass * external_force(particle.position);

            particle.tempPosition = particle.position;
            particle.oldPosition = particle.position;
      }

      damp_velocities();

      for (EntityId entity : m_entities)
      {
            auto& particle = get_particle(entity);
            particle.position = particle.position + dt * particle.velocity;

            sync_grid(particle, entity);
      }

      m_profiler.start_measure("gen col const");
      generate_collision_constraints();
      logger::log("gen col const", m_profiler.end_measure("gen col const"));

      m_profiler.start_measure("solve const");
      solve_constraints();
      logger::log("solve const: ", m_profiler.end_measure("solve const"));

      for (EntityId entity : m_entities)
      {
            auto& particle = get_particle(entity);
            particle.velocity = (particle.position - particle.oldPosition) / dt;
      
            sync_grid(particle, entity);
      }

      velocity_update();

      // sync transform
      for (EntityId entity : m_entities)
            m_ecs->get_component<Transform>(entity).position = Vector3(get_particle(entity).position, 0);

      // clear collision constraints
      m_constraints.erase(m_constraints.begin() + m_collisionConstraintStart, m_constraints.end());
}

// ---------------------------------------
// PRIVATE MEHODS
// ---------------------------------------

void PBDSystem::damp_velocities()
{

}
void PBDSystem::velocity_update()
{

}
void PBDSystem::generate_collision_constraints()
{
      m_collisionConstraintStart = m_constraints.size();

      std::vector<EntityId> surroundingParticles;
      std::sort(m_entities.begin(), m_entities.end());
      for (EntityId entity : m_entities)
      {
            const auto& particle = get_particle(entity);
            if (particle.radius == 0)
                  continue;

            surroundingParticles.clear();

            m_grid.surrounding_particles(particle.position, surroundingParticles);

            for (const auto& surroundingParticle : surroundingParticles)
            {
                  if (entity >= surroundingParticle)
                        continue;

                  const auto& other = get_particle(surroundingParticle);
                  if (other.radius == 0)
                        continue;

                  auto constraint = add_constraint<CollisionConstraint>(
                      { entity, surroundingParticle },
                      Inequality
                  );

                  constraint->m_distance = particle.radius + get_particle(surroundingParticle).radius;
                  constraint->m_stiffness = 0.1f;
            }
      }
}
void PBDSystem::solve_constraints()
{
      solve_seidel_gauss();
      // solve_sys();
}
void PBDSystem::solve_seidel_gauss()
{
      // premature optimization YAY
      std::vector<PBDParticle*> particles;
      std::vector<Vec> deltas;
      for (int i = 0; i < m_solverIterations; i++)
      {
            // project constraints
            for (Constraint* constraint : m_constraints)
            {
                  particles.clear();
                  for (auto eId : constraint->m_entities)
                        particles.push_back(&get_particle(eId));

                  float acc = 0; Vec g;
                  for (size_t j = 0; j < particles.size(); j++)
                  {
                        g = constraint->constraint_gradient(j, particles);
                        acc += particles[j]->invmass * glm::dot(g, g);
                  }
                  //if (acc < 0.01f && acc >= 0.f)
                  //      acc = 0.01f;
                  //if (acc > -0.01f && acc < 0.f)
                  //      acc = -0.01f;
                  if (acc == 0.f)
                        continue;

                  float constraintErr = constraint->constraint(particles);
                  if (
                        constraint->m_type == Inequality && constraintErr >= 0
                        || constraint->m_type == InverseInequality && constraintErr <= 0
                  )
                        continue;

                  float scalingFactor = constraintErr / acc;

                  deltas.clear();
                  for (size_t j = 0; j < particles.size(); j++)
                  {
                        deltas.push_back(
                              -scalingFactor
                              * particles[j]->invmass
                              * constraint->constraint_gradient(j, particles)
                        );
                  }
                  float correctedStiffness = 1 - std::pow(1 - constraint->m_stiffness, constraint->m_cardinality);
                  for (size_t j = 0; j < particles.size(); j++)
                  {
                        Vec delta = correctedStiffness * deltas[j];
                        //if (delta != delta)
                        //      delta = Vec(0);
                        particles[j]->position += delta;
                        
                  }
            }
      }
}
void PBDSystem::solve_sys()
{

}

PBDParticle& PBDSystem::get_particle(EntityId id)
{
      return m_ecs->get_component<PBDParticle>(id);
}
Vec PBDSystem::external_force(Vec pos)
{
      return Vector2(0, 9.81f);
}
void PBDSystem::sync_grid(PBDParticle& particle, EntityId entity)
{
      m_grid.change_particle(particle.tempPosition, particle.position, entity);
      particle.tempPosition = particle.position;
}

// ---------------------------------------
// CONSTRAINT
// ---------------------------------------

Constraint::Constraint(Cardinality cardinality, std::vector<EntityId> entities) :
      m_cardinality { cardinality }, m_entities{ entities }, m_stiffness{ 1.f }, m_type{ Equality }
{}

// ---------------------------------------
// COLLISION CONSTRAINT
// ---------------------------------------

CollisionConstraint::CollisionConstraint(float distance, std::vector<EntityId> entities) :
      Constraint(2, entities), m_distance{ distance }
{}
float CollisionConstraint::constraint(InParticles particles)
{
      return glm::length(particles[0]->position - particles[1]->position) - m_distance;
}
Vec CollisionConstraint::constraint_gradient(size_t der, InParticles particles)
{
      Vec d = particles[0]->position - particles[1]->position;
      float length = glm::length(d);
      if (length != 0)
            d /= length;
      return d * (static_cast<float>(der) * -2.f + 1.f);
}
