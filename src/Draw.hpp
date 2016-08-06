#pragma once

//
// 単純な図形の描画
// 


namespace ngs { namespace Draw {

// 塗り潰し円弧
// center_x, center_y  円の中心位置
// radius              半径
// start_rad, end_rad  開始・終了角度
// division            円の分割数(数値が大きいと滑らかな円になる)
// color               色
void fillArc(const float center_x, const float center_y,
             const float radius,
             const float start_rad, const float end_rad,
             const int division) {
  // 頂点データを生成
  std::vector<GLfloat> vtx;
  vtx.reserve(division * 2);                        // TIPS:正確な値である必要はない

  vtx.push_back(center_x);
  vtx.push_back(center_y);

  for (int i = 0; i <= division; ++i) {
    float r = ((end_rad - start_rad) * i) / division + start_rad;

    vtx.push_back(radius * std::sin(r) + center_x);
    vtx.push_back(radius * std::cos(r) + center_y);
  }

	auto ctx = ci::gl::context();
	const auto* curGlslProg = ctx->getGlslProg();
	if( ! curGlslProg ) {
		DOUT << "No GLSL program bound" << std::endl;
		return;
	}

	ctx->pushVao();
	ctx->getDefaultVao()->replacementBindBegin();

	auto defaultVbo = ctx->getDefaultArrayVbo(vtx.size() * sizeof(GLfloat));
  ci::gl::ScopedBuffer vboScp( defaultVbo );
  
	int posLoc = curGlslProg->getAttribSemanticLocation( ci::geom::Attrib::POSITION );
  assert(posLoc >= 0);
  ci::gl::enableVertexAttribArray( posLoc );
  ci::gl::vertexAttribPointer( posLoc, 2, GL_FLOAT, GL_FALSE, 0, 0 );

	defaultVbo->bufferSubData( 0, vtx.size() * sizeof(GLfloat), &vtx[0] );
	ctx->getDefaultVao()->replacementBindEnd();

	ctx->setDefaultShaderVars();
	ctx->drawArrays( GL_TRIANGLE_FAN, 0, vtx.size() / 2 );
	ctx->popVao();
}

} }
