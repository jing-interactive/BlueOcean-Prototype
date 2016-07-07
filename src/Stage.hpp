#pragma once

//
// 広大なステージ
//  高さ情報を持つだけでは表現力に限界がある
//  効率的に複雑な地形を表現するには..
//

#include <vector>
#include <cinder/Rand.h>
#include <cinder/Color.h>
#include <cinder/TriMesh.h>
#include <cinder/Perlin.h>


namespace ngs {

class Stage {
  ci::TriMesh mesh_;

  
public:
  // FIXME:奥行きはdeepなのか??
  Stage(const int width, const int deep,
        const int octerve, const int seed,
        const float random_scale, const float height_scale) {
    // 高さ情報を格納する
    std::vector<std::vector<int> > height_map;

    height_map.resize(deep);
    for (auto& row : height_map) {
      row.resize(width);
    }

    // パーリンノイズを使っていい感じに地形の起伏を生成
    ci::Perlin random(octerve, seed);
    
    for (u_int z = 0; z < deep; ++z) {
      for (u_int x = 0; x < width; ++x) {
        height_map[z][x] = glm::clamp(random.fBm(x * random_scale, z * random_scale) * height_scale,
                                      0.0f, 15.0f);
      }
    }

    // 高さ情報を元にTriMeshを生成
    // 隣のブロックの高さを調べ、自分より低ければその分壁を作る作戦
    uint32_t index = 0;
    for (int z = 0; z < deep; ++z) {
      for (int x = 0; x < width; ++x) {
        float y = height_map[z][x];
        
        {
          // 上面
          ci::vec3 p[] = {
            {     x, y, z },
            { x + 1, y, z },
            {     x, y, z + 1 },
            { x + 1, y, z + 1 },
          };

          ci::vec3 n[] = {
            { 0, 1, 0 },
            { 0, 1, 0 },
            { 0, 1, 0 },
            { 0, 1, 0 },
          };
          
          ci::vec2 uv[] = {
            { 0, p[0].y / 16.0f },
            { 0, p[1].y / 16.0f },
            { 0, p[2].y / 16.0f },
            { 0, p[3].y / 16.0f },
          };
        
          mesh_.appendPositions(&p[0], 4);
          mesh_.appendNormals(&n[0], 4);
          mesh_.appendTexCoords0(&uv[0], 4);
        
          mesh_.appendTriangle(index + 0, index + 2, index + 1);
          mesh_.appendTriangle(index + 1, index + 2, index + 3);
          index += 4;
        }

        if ((z > 0) && (height_map[z - 1][x] < y)) {
          // 側面(z-)
          int dy = y - height_map[z - 1][x];
          for (int h = 0; h < dy; ++h) {
            ci::vec3 p[] = {
              {     x,     y - h, z },
              { x + 1,     y - h, z },
              {     x, y - 1 - h, z },
              { x + 1, y - 1 - h, z },
            };
          
            ci::vec3 n[] = {
              { 0, 0, -1 },
              { 0, 0, -1 },
              { 0, 0, -1 },
              { 0, 0, -1 },
            };

            ci::vec2 uv[] = {
              { 0, p[0].y / 16.0f },
              { 0, p[0].y / 16.0f },
              { 0, p[0].y / 16.0f },
              { 0, p[0].y / 16.0f },
            };
          
            mesh_.appendPositions(&p[0], 4);
            mesh_.appendNormals(&n[0], 4);
            mesh_.appendTexCoords0(&uv[0], 4);
        
            mesh_.appendTriangle(index + 0, index + 1, index + 2);
            mesh_.appendTriangle(index + 1, index + 3, index + 2);
            index += 4;
          }
        }
        
        if ((z < (deep - 1)) && (height_map[z + 1][x] < y)) {
          // 側面(z+)
          int dy = y - height_map[z + 1][x];
          for (int h = 0; h < dy; ++h) {
            ci::vec3 p[] = {
              {     x,     y - h, z + 1 },
              { x + 1,     y - h, z + 1 },
              {     x, y - 1 - h, z + 1 },
              { x + 1, y - 1 - h, z + 1 },
            };
          
            ci::vec3 n[] = {
              { 0, 0, 1 },
              { 0, 0, 1 },
              { 0, 0, 1 },
              { 0, 0, 1 },
            };

            ci::vec2 uv[] = {
              { 0, p[0].y / 16.0f },
              { 0, p[0].y / 16.0f },
              { 0, p[0].y / 16.0f },
              { 0, p[0].y / 16.0f },
            };
          
            mesh_.appendPositions(&p[0], 4);
            mesh_.appendNormals(&n[0], 4);
            mesh_.appendTexCoords0(&uv[0], 4);
        
            mesh_.appendTriangle(index + 0, index + 3, index + 1);
            mesh_.appendTriangle(index + 0, index + 2, index + 3);
            index += 4;
          }
        }
        
        if ((x > 0) && (height_map[z][x - 1] < y)) {
          // 側面(x-)
          int dy = y - height_map[z][x - 1];
          for (int h = 0; h < dy; ++h) {
            ci::vec3 p[] = {
              { x,     y - h,     z },
              { x,     y - h, z + 1 },
              { x, y - 1 - h,     z },
              { x, y - 1 - h, z + 1 },
            };
          
            ci::vec3 n[] = {
              { -1, 0, 0 },
              { -1, 0, 0 },
              { -1, 0, 0 },
              { -1, 0, 0 },
            };

            ci::vec2 uv[] = {
              { 0, p[0].y / 16.0f },
              { 0, p[0].y / 16.0f },
              { 0, p[0].y / 16.0f },
              { 0, p[0].y / 16.0f },
            };
          
            mesh_.appendPositions(&p[0], 4);
            mesh_.appendNormals(&n[0], 4);
            mesh_.appendTexCoords0(&uv[0], 4);
        
            mesh_.appendTriangle(index + 0, index + 2, index + 1);
            mesh_.appendTriangle(index + 1, index + 2, index + 3);
            index += 4;
          }
        }
        
        if ((x < (width - 1)) && (height_map[z][x + 1] < y)) {
          // 側面(x+)
          int dy = y - height_map[z][x + 1];
          for (int h = 0; h < dy; ++h) {
            ci::vec3 p[] = {
              { x + 1,     y - h,     z },
              { x + 1,     y - h, z + 1 },
              { x + 1, y - 1 - h,     z },
              { x + 1, y - 1 - h, z + 1 },
            };
          
            ci::vec3 n[] = {
              { 1, 0, 0 },
              { 1, 0, 0 },
              { 1, 0, 0 },
              { 1, 0, 0 },
            };

            ci::vec2 uv[] = {
              { 0, p[0].y / 16.0f },
              { 0, p[0].y / 16.0f },
              { 0, p[0].y / 16.0f },
              { 0, p[0].y / 16.0f },
            };
          
            mesh_.appendPositions(&p[0], 4);
            mesh_.appendNormals(&n[0], 4);
            mesh_.appendTexCoords0(&uv[0], 4);
        
            mesh_.appendTriangle(index + 0, index + 1, index + 3);
            mesh_.appendTriangle(index + 0, index + 3, index + 2);
            index += 4;
          }
        }
      }
    }
  }

  const ci::TriMesh& mesh() const {
    return mesh_;
  }

};

}
