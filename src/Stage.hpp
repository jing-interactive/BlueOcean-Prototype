#pragma once

//
// 広大なステージ
//  高さ情報を持つだけでは表現力に限界がある
//  効率的に複雑な地形を表現するには..
//

#include <vector>
#include <cinder/Perlin.h>
#include <cinder/Color.h>
#include <cinder/TriMesh.h>
#include <cinder/AxisAlignedBox.h>


namespace ngs {

class Stage {
  ci::ivec2 size_;

  std::vector<std::vector<int> > height_map_;
  
  ci::TriMesh land_;
  ci::AxisAlignedBox aabb_;

  
public:
  // FIXME:奥行きはdeepなのか??
  Stage(const int width, const int deep,
        const int offset_x, const int offset_z,
        const ci::Perlin& random,
        const float random_scale, const float height_scale)
    : size_(width, deep)
  {
    height_map_.resize(deep + 2);
    for (auto& row : height_map_) {
      row.resize(width + 2);
    }

    // パーリンノイズを使っていい感じに地形の起伏を生成
    // 周囲１ブロックを余計に生成している
    for (int z = -1; z < (deep + 1); ++z) {
      for (int x = -1; x < (width + 1); ++x) {
        height_map_[z + 1][x + 1] = glm::clamp(random.fBm((x + offset_x * width) * random_scale,
                                                         (z + offset_z * deep) * random_scale) * height_scale,
                                              -15.0f, 15.0f);
      }
    }

    // 高さ情報を元にTriMeshを生成
    // 隣のブロックの高さを調べ、自分より低ければその分壁を作る作戦
    // TODO:コピペ感をなくす
    uint32_t index = 0;
    for (int z = 0; z < deep; ++z) {
      for (int x = 0; x < width; ++x) {
        float y = height_map_[z + 1][x + 1];
        
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
        
          land_.appendPositions(&p[0], 4);
          land_.appendNormals(&n[0], 4);
          land_.appendTexCoords0(&uv[0], 4);
        
          land_.appendTriangle(index + 0, index + 2, index + 1);
          land_.appendTriangle(index + 1, index + 2, index + 3);
          index += 4;
        }

        if ((height_map_[z - 1 + 1][x + 1] < y)) {
          // 側面(z-)
          int dy = y - height_map_[z - 1 + 1][x + 1];
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
          
            land_.appendPositions(&p[0], 4);
            land_.appendNormals(&n[0], 4);
            land_.appendTexCoords0(&uv[0], 4);
        
            land_.appendTriangle(index + 0, index + 1, index + 2);
            land_.appendTriangle(index + 1, index + 3, index + 2);
            index += 4;
          }
        }
        
        if ((height_map_[z + 1 + 1][x + 1] < y)) {
          // 側面(z+)
          int dy = y - height_map_[z + 1 + 1][x + 1];
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
          
            land_.appendPositions(&p[0], 4);
            land_.appendNormals(&n[0], 4);
            land_.appendTexCoords0(&uv[0], 4);
        
            land_.appendTriangle(index + 0, index + 3, index + 1);
            land_.appendTriangle(index + 0, index + 2, index + 3);
            index += 4;
          }
        }
        
        if ((height_map_[z + 1][x - 1 + 1] < y)) {
          // 側面(x-)
          int dy = y - height_map_[z + 1][x - 1 + 1];
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
          
            land_.appendPositions(&p[0], 4);
            land_.appendNormals(&n[0], 4);
            land_.appendTexCoords0(&uv[0], 4);
        
            land_.appendTriangle(index + 0, index + 2, index + 1);
            land_.appendTriangle(index + 1, index + 2, index + 3);
            index += 4;
          }
        }
        
        if ((height_map_[z + 1][x + 1 + 1] < y)) {
          // 側面(x+)
          int dy = y - height_map_[z + 1][x + 1 + 1];
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
          
            land_.appendPositions(&p[0], 4);
            land_.appendNormals(&n[0], 4);
            land_.appendTexCoords0(&uv[0], 4);
        
            land_.appendTriangle(index + 0, index + 1, index + 3);
            land_.appendTriangle(index + 0, index + 3, index + 2);
            index += 4;
          }
        }
      }
    }
    aabb_ = land_.calcBoundingBox();
  }

  
  const std::vector<std::vector<int> >& getHeightMap() const {
    return height_map_;
  }
  
  const ci::TriMesh& getLandMesh() const {
    return land_;
  }

  const ci::AxisAlignedBox& getAABB() const {
    return aabb_;
  }
  
  const ci::ivec2& getSize() const {
    return size_;
  }

};

}
