

// camera, lighting, matrices, geometry, heirachical, components


class Entity {
   public:
      void observe(String * name, void(*action)(const void *));
      void fire(String * name, void * data);
      
      virtual void step();
      virtual void draw();
      
      glm::vec3 pos;
      glm::vec3 vel;
      double scale;
      
      Entity(String *meshName);
      
      void destroy() {
         
      }
   private:
      int id;
      
      static std::array<Mesh> loadedMeshes;
      static std::array<Entity> freeList;
}

class Behavior {
public:
   Behavior(String evt) event(evet) {};
}

class DestroyedOnCollision : Behavior {
public:
   DestroyedOnCollision() { Behavior('collide'); }
   void onCollide(void *data) {
      delete this;
   }
}

class GrowsOnCollision : Behavior {
public:
   GrowsOnCollision() { Behavior('collide', onCollide); }
   
private:
   void onCollide(void *data) {
      Entity *ent = data;
      ent->scale *= 1.1;
   }
}

class FireHydrant : Entity, DestroyedOnCollision {
  // mesh mod
}

class TRex : Entity, GrowsOnCollision {

}


