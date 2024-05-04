#pragma once

#include <string>
#include <unordered_map>

#include "reference.h"

class Dependency;

// typedef Reference<Dependency> DependencyRef;
typedef std::string typeName;
typedef struct DependencyRef_T
{
      Reference<Dependency> dependency;
      typeName type;
} DependencyRef;

template<typename T>
inline typeName type_to_name()
{
      return typeid(T).name();
}

template<typename T>
inline DependencyRef make_dependency_ref(Reference<T> object)
{
      DependencyRef ref = {};
      ref.type = type_to_name<T>();
      ref.dependency = Reference<Dependency>(object.get());
      return ref;
}

class Dependency
{
public:

      Dependency();
      bool try_update();
      bool resolved() const;

      template<typename T>
      void add_dependency(REF(T) dependency) { add_dependency_ref(make_dependency_ref(dependency)); }
      template<typename T>
      void add_dependent(REF(T) dependent) { add_dependent_ref(make_dependency_ref(dependent)); }
      template<typename T>
      void add_dependencies(std::initializer_list<REF(T)> dependencies)
      {
            std::vector<DependencyRef> deps;
            for (auto dep : dependencies)
                  add_dependency_ref(make_dependency_ref(dep));
      }

protected:

      virtual void on_update() = 0;

      std::unordered_map<typeName, std::vector<DependencyRef>> m_dependencies;
      std::unordered_map<typeName, std::vector<DependencyRef>> m_dependents;

      template<typename T>
      inline Reference<T> get_dependency()
      {
            return dynamic_cast<T*>(get_dependencies().first().get());
      }
      template<typename T>
      inline std::vector<Reference<T>> get_dependencies()
      {
            return m_dependencies[type_to_name<T>()];
      }

private:

      void resolve();
      bool m_resolved;

      void push_to_map(std::unordered_map<typeName, std::vector<DependencyRef>>& map, DependencyRef dependency);

      void add_dependency_ref(DependencyRef dependency);
      void add_dependent_ref(DependencyRef dependent);

};