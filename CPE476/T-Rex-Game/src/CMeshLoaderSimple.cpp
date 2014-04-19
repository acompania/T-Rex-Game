#include "CMeshLoaderSimple.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <math.h>
#include <iomanip>
#include <string>
#include <streambuf>
#include "glm/glm.hpp"

using namespace std;

class SColor
{

public:

    float Red, Green, Blue;

    SColor()
        : Red(0.7f), Green(0.7f), Blue(0.7f)
    {}

};

class SVector3
{

public:

   float X, Y, Z;

   SVector3()
      : X(0), Y(0), Z(0)
   {}

   SVector3(float in)
      : X(in), Y(in), Z(in)
   {}

   SVector3(float in_x, float in_y, float in_z)
      : X(in_x), Y(in_y), Z(in_z)
   {}

   SVector3 crossProduct(SVector3 const & v) const
   {
      return SVector3(Y*v.Z - v.Y*Z, v.X*Z - X*v.Z, X*v.Y - v.X*Y);
   }

   float dotProduct(SVector3 const & v) const
   {
      return X*v.X + Y*v.Y + Z*v.Z;
   }

   float length() const
   {
      return sqrtf(X*X + Y*Y + Z*Z);
   }

   // Added this
   SVector3 normalize() const
   {
      return (*this)/(this->length());
   }

   SVector3 operator + (SVector3 const & v) const
   {
      return SVector3(X+v.X, Y+v.Y, Z+v.Z);
   }

   SVector3 & operator += (SVector3 const & v)
   {
      X += v.X;
      Y += v.Y;
      Z += v.Z;

      return * this;
   }

   SVector3 operator - (SVector3 const & v) const
   {
      return SVector3(X-v.X, Y-v.Y, Z-v.Z);
   }

   SVector3 & operator -= (SVector3 const & v)
   {
      X -= v.X;
      Y -= v.Y;
      Z -= v.Z;

      return * this;
   }

   SVector3 operator * (SVector3 const & v) const
   {
      return SVector3(X*v.X, Y*v.Y, Z*v.Z);
   }

   SVector3 & operator *= (SVector3 const & v)
   {
      X *= v.X;
      Y *= v.Y;
      Z *= v.Z;

      return * this;
   }

   SVector3 operator / (SVector3 const & v) const
   {
      return SVector3(X/v.X, Y/v.Y, Z/v.Z);
   }

   SVector3 & operator /= (SVector3 const & v)
   {
      X /= v.X;
      Y /= v.Y;
      Z /= v.Z;

      return * this;
   }

   SVector3 operator * (float const s) const
   {
      return SVector3(X*s, Y*s, Z*s);
   }

   SVector3 & operator *= (float const s)
   {
      X *= s;
      Y *= s;
      Z *= s;

      return * this;
   }

   SVector3 operator / (float const s) const
   {
      return SVector3(X/s, Y/s, Z/s);
   }

   SVector3 & operator /= (float const s)
   {
      X /= s;
      Y /= s;
      Z /= s;

      return * this;
   }

};

class SVertex
{

public:

    SVector3 Position;
    SVector3 Normal;
    SColor Color;

};

class CMesh
{

    friend class CVertexBufferObject;
    friend class CMeshLoader;

    struct STriangle
    {
        unsigned int VertexIndex1, VertexIndex2, VertexIndex3;
      SColor Color;
    };

    std::vector<SVertex> Vertices;
    std::vector<SVertex> Normals;
    std::vector<STriangle> Triangles;

    CMesh();

public:

    ~CMesh();

   void centerMeshByAverage(SVector3 const & CenterLocation);
   void centerMeshByExtents(SVector3 const & CenterLocation);

   void resizeMesh(SVector3 const & Scale);

    void computeNormals();

};

bool CMeshLoader::loadVertexBufferObjectFromMesh(std::string const & fileName, int & TriangleCount, GLuint & PositionBufferHandle, GLuint & ColorBufferHandle, GLuint & NormalBufferHandle)
{
   CMesh * Mesh = loadASCIIMesh(fileName);
   if (! Mesh)
      return false;

   Mesh->resizeMesh(SVector3(1));
   Mesh->centerMeshByExtents(SVector3(0));
   Mesh->computeNormals();

   createVertexBufferObject(* Mesh, TriangleCount, PositionBufferHandle, ColorBufferHandle, NormalBufferHandle);
   
   return true;
}
 
CMesh * const CMeshLoader::loadASCIIMesh(std::string const & fileName) {
   std::ifstream File;
   File.open(fileName.c_str());

   if (! File.is_open()) {
      std::cerr << "Unable to open mesh file: " << fileName << std::endl;
      return 0;
   }

   CMesh * Mesh = new CMesh();

   // find the file type
   int dotPos = fileName.find_last_of(".");
   
   if (fileName.find(".m", dotPos) != string::npos) {
      while (File) {
         std::string ReadString;
         std::getline(File, ReadString);

         std::stringstream Stream(ReadString);

         std::string Label;
         Stream >> Label;

         if (Label.find("#") != std::string::npos) {
            continue; // Comment, skip
         }

         if ("Vertex" == Label) {
            int Index;
            Stream >> Index; // We don't care, throw it away

            SVector3 Position;
            Stream >> Position.X;
            Stream >> Position.Y;
            Stream >> Position.Z;

            SVertex Vertex;
            Vertex.Position = Position;
            Vertex.Normal = SVector3(0);

            Mesh->Vertices.push_back(Vertex);
         }
         else if ("Face" == Label) {
            int Index;
            Stream >> Index; // We don't care, throw it away

            int Vertex1, Vertex2, Vertex3;
            Stream >> Vertex1;
            Stream >> Vertex2;
            Stream >> Vertex3;

            CMesh::STriangle Triangle;
            Triangle.VertexIndex1 = Vertex1 - 1;
            Triangle.VertexIndex2 = Vertex2 - 1;
            Triangle.VertexIndex3 = Vertex3 - 1;

            // there is a color
            size_t Location;
            if ((Location = ReadString.find("{")) != std::string::npos) {
               Location = ReadString.find("rgb=(");
               Location += 5; // rgb=( is 5 characters

               ReadString = ReadString.substr(Location);
               std::stringstream Stream(ReadString);
               float Color;
               Stream >> Color;
               Triangle.Color.Red = Color;
               Stream >> Color;
               Triangle.Color.Green = Color;
               Stream >> Color;
               Triangle.Color.Blue = Color;
            }

            // Add the normal to the current face
            SVector3 vert1 = (Mesh->Vertices)[Triangle.VertexIndex1].Position;
            SVector3 vert2 = (Mesh->Vertices)[Triangle.VertexIndex2].Position;
            SVector3 vert3 = (Mesh->Vertices)[Triangle.VertexIndex3].Position;
            SVector3 FaceNormal = (vert2 - vert1).crossProduct(vert3 - vert1);

            // Add the normal to each vertex of the face
            (Mesh->Vertices)[Triangle.VertexIndex1].Normal += FaceNormal;
            (Mesh->Vertices)[Triangle.VertexIndex2].Normal += FaceNormal;
            (Mesh->Vertices)[Triangle.VertexIndex3].Normal += FaceNormal;
            // printf("%f %f %f\n", (Mesh->Vertices)[Triangle.VertexIndex1].Normal.X, (Mesh->Vertices)[Triangle.VertexIndex1].Normal.Y, (Mesh->Vertices)[Triangle.VertexIndex1].Normal.Z);

            Mesh->Triangles.push_back(Triangle);
         }
         else if ("" == Label) { /* new line */ }
         else if ("Corner" == Label) { /* no normals */ }
         else {
            std::cerr << "While parsing .m ASCII mesh: Expected 'Vertex' or 'Face' label, found '" << Label << "'." << std::endl;
         }
      }

      if (! Mesh->Triangles.size() || ! Mesh->Vertices.size()) {
         delete Mesh;
         printf("No Triangles or Vertices\n");
         return 0;
      }
   }
   else if (fileName.find(".obj", dotPos) != string::npos) {
      std::string ReadString;

      int first = 1;
      while (File && std::getline(File, ReadString)) {
         std::stringstream Stream(ReadString);

         std::string Label;
         Stream >> Label;

         if ("g" == Label || "o" == Label) {
            if (!first)
               return Mesh;
            first = 0;
         }

         if (Label.find("#") != std::string::npos) { 
            continue;
         }

         if ("mtllib" == Label || "o" == Label || "usemtl" == Label || "s" == Label ||
               "vn" == Label || "g" == Label || "vt" == Label || "l" == Label) {
            continue;
         }

         if ("v" == Label) {
            SVector3 Position;
            Stream >> Position.X;
            Stream >> Position.Y;
            Stream >> Position.Z;

            SVertex Vertex;
            Vertex.Position = Position;
            Vertex.Normal = SVector3(0);

            Mesh->Vertices.push_back(Vertex);
         }
         else if ("f" == Label) {
            int Vertex1, Vertex2, Vertex3;
            Stream >> Vertex1 >> Vertex2 >> Vertex3;
            //std::cerr << Vertex1 << " " << Vertex2 << " " << Vertex3 << std::endl;


            CMesh::STriangle Triangle;
            Triangle.VertexIndex1 = Vertex1-1;
            Triangle.VertexIndex2 = Vertex2-1;
            Triangle.VertexIndex3 = Vertex3-1;

            // Add the normal to the current face
            SVector3 vert1 = (Mesh->Vertices)[Triangle.VertexIndex1].Position;
            SVector3 vert2 = (Mesh->Vertices)[Triangle.VertexIndex2].Position;
            SVector3 vert3 = (Mesh->Vertices)[Triangle.VertexIndex3].Position;
            SVector3 FaceNormal = (vert2 - vert1).crossProduct(vert3 - vert1);

            // Add the normal to each vertex of the face
            (Mesh->Vertices)[Triangle.VertexIndex1].Normal += FaceNormal;
            (Mesh->Vertices)[Triangle.VertexIndex2].Normal += FaceNormal;
            (Mesh->Vertices)[Triangle.VertexIndex3].Normal += FaceNormal;
            // printf("%f %f %f\n", (Mesh->Vertices)[Triangle.VertexIndex1].Normal.X, (Mesh->Vertices)[Triangle.VertexIndex1].Normal.Y, (Mesh->Vertices)[Triangle.VertexIndex1].Normal.Z);

            Mesh->Triangles.push_back(Triangle);
         }
         else {
            std::cerr << "While parsing .obj ASCII mesh: Expected 'v' or 'f' label, found '" << Label << "'." << std::endl;
         }
      }

      if (! Mesh->Triangles.size() || ! Mesh->Vertices.size()) {
         delete Mesh;
         return 0;
      }
   }
   else {
      std::cerr << "Invalid file type (must be either .m or .obj)" << std::endl;
      return 0;
   }

   return Mesh;
}

void CMeshLoader::createVertexBufferObject(CMesh const & Mesh, int & TriangleCount, GLuint & PositionBufferHandle, GLuint & ColorBufferHandle, GLuint & NormalBufferHandle)
{
   glGenBuffers(1, & PositionBufferHandle);
   glGenBuffers(1, & ColorBufferHandle);
   glGenBuffers(1, & NormalBufferHandle);

   std::vector<GLfloat> VertexPositions;
   std::vector<GLfloat> Colors;
   std::vector<GLfloat> lNormals;

   for(unsigned int j = 0; j < Mesh.Triangles.size(); j++) 
   {
          SVertex Vertex;

        Vertex = Mesh.Vertices[Mesh.Triangles[j].VertexIndex1];

        VertexPositions.push_back(Vertex.Position.X);
        VertexPositions.push_back(Vertex.Position.Y);
        VertexPositions.push_back(Vertex.Position.Z);
      VertexPositions.push_back(1.f);
      Colors.push_back(Mesh.Triangles[j].Color.Red);
      Colors.push_back(Mesh.Triangles[j].Color.Green);
      Colors.push_back(Mesh.Triangles[j].Color.Blue);
      lNormals.push_back(Vertex.Normal.X);
      lNormals.push_back(Vertex.Normal.Y);
      lNormals.push_back(Vertex.Normal.Z);

      Vertex = Mesh.Vertices[Mesh.Triangles[j].VertexIndex2];

        VertexPositions.push_back(Vertex.Position.X);
        VertexPositions.push_back(Vertex.Position.Y);
        VertexPositions.push_back(Vertex.Position.Z);
      VertexPositions.push_back(1.f);
      Colors.push_back(Mesh.Triangles[j].Color.Red);
      Colors.push_back(Mesh.Triangles[j].Color.Green);
      Colors.push_back(Mesh.Triangles[j].Color.Blue);
      lNormals.push_back(Vertex.Normal.X);
      lNormals.push_back(Vertex.Normal.Y);
      lNormals.push_back(Vertex.Normal.Z);
        
      Vertex = Mesh.Vertices[Mesh.Triangles[j].VertexIndex3];

        VertexPositions.push_back(Vertex.Position.X);
        VertexPositions.push_back(Vertex.Position.Y);
        VertexPositions.push_back(Vertex.Position.Z);
      VertexPositions.push_back(1.f);
      Colors.push_back(Mesh.Triangles[j].Color.Red);
      Colors.push_back(Mesh.Triangles[j].Color.Green);
      Colors.push_back(Mesh.Triangles[j].Color.Blue);
      lNormals.push_back(Vertex.Normal.X);
      lNormals.push_back(Vertex.Normal.Y);
      lNormals.push_back(Vertex.Normal.Z);
   }

    TriangleCount = Mesh.Triangles.size();

   glBindBuffer(GL_ARRAY_BUFFER, PositionBufferHandle);
   glBufferData(GL_ARRAY_BUFFER, VertexPositions.size()*sizeof(GLfloat), & VertexPositions.front(), GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, ColorBufferHandle);
   glBufferData(GL_ARRAY_BUFFER, Colors.size()*sizeof(GLfloat), & Colors.front(), GL_STATIC_DRAW);
   
    glBindBuffer(GL_ARRAY_BUFFER, NormalBufferHandle);
   glBufferData(GL_ARRAY_BUFFER, lNormals.size()*sizeof(GLfloat), & lNormals.front(), GL_STATIC_DRAW);
}

void CMesh::computeNormals() {
    for (std::vector<SVertex>::iterator it = Vertices.begin(); it != Vertices.end(); ++ it) {
      it->Normal = it->Normal.normalize();
    }
}

CMesh::CMesh()
{}

CMesh::~CMesh()
{}

void CMesh::centerMeshByAverage(SVector3 const & CenterLocation)
{
   SVector3 VertexSum;
   for (std::vector<SVertex>::const_iterator it = Vertices.begin(); it != Vertices.end(); ++ it)
      VertexSum += it->Position;

   VertexSum /= (float) Vertices.size();
   SVector3 VertexOffset = CenterLocation - VertexSum;
   for (std::vector<SVertex>::iterator it = Vertices.begin(); it != Vertices.end(); ++ it)
      it->Position += VertexOffset;
}

void CMesh::centerMeshByExtents(SVector3 const & CenterLocation)
{
   if (Vertices.size() < 2)
      return;

   SVector3 Min, Max;
   {
      std::vector<SVertex>::const_iterator it = Vertices.begin();
      Min = it->Position;
      Max = it->Position;
      for (; it != Vertices.end(); ++ it)
      {
         if (Min.X > it->Position.X)
            Min.X = it->Position.X;
         if (Min.Y > it->Position.Y)
            Min.Y = it->Position.Y;
         if (Min.Z > it->Position.Z)
            Min.Z = it->Position.Z;

         if (Max.X < it->Position.X)
            Max.X = it->Position.X;
         if (Max.Y < it->Position.Y)
            Max.Y = it->Position.Y;
         if (Max.Z < it->Position.Z)
            Max.Z = it->Position.Z;
      }
   }

   SVector3 Center = (Max + Min) / 2;

   SVector3 VertexOffset = CenterLocation - Center;
   for (std::vector<SVertex>::iterator it = Vertices.begin(); it != Vertices.end(); ++ it)
      it->Position += VertexOffset;
}



void CMesh::resizeMesh(SVector3 const & Scale)
{
   if (Vertices.size() < 2)
      return;

   SVector3 Min, Max;
   {
      std::vector<SVertex>::const_iterator it = Vertices.begin();
      Min = it->Position;
      Max = it->Position;
      for (; it != Vertices.end(); ++ it)
      {
         if (Min.X > it->Position.X)
            Min.X = it->Position.X;
         if (Min.Y > it->Position.Y)
            Min.Y = it->Position.Y;
         if (Min.Z > it->Position.Z)
            Min.Z = it->Position.Z;

         if (Max.X < it->Position.X)
            Max.X = it->Position.X;
         if (Max.Y < it->Position.Y)
            Max.Y = it->Position.Y;
         if (Max.Z < it->Position.Z)
            Max.Z = it->Position.Z;
      }
   }

   SVector3 Extent = (Max - Min);
   SVector3 Resize = Scale / std::max(Extent.X, std::max(Extent.Y, Extent.Z));
   for (std::vector<SVertex>::iterator it = Vertices.begin(); it != Vertices.end(); ++ it)
      it->Position *= Resize;
}
