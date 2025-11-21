#include <iostream>

#include "Maths.h"
#include "GamesEngineeringBase.h"


template<typename t>
t simpleInterpolateAttribute(t a0, t a1, t a2, float alpha, float beta, float gamma) {
	return (a0 * alpha) + (a1 * beta) + (a2 * gamma);
}

template<typename t>
t perspectiveCorrectInterpolateAttribute(t a0, t a1, t a2, float v0_w, float v1_w, float v2_w, float alpha, float beta, float gamma, float frag_w)
{
	
	t attrib[3];
	attrib[0] = a0 * alpha * v0_w;
	attrib[1] = a1 * beta * v1_w;
	attrib[2] = a2 * gamma * v2_w;
	return ((attrib[0] + attrib[1] + attrib[2]) / frag_w);
}

class Triangle {
public:
	/*Vec4 v0;
	Vec4 v1;
	Vec4 v2;*/
	
	/*Triangle() 
		: v0(0.0f, 0.0f, 0.0f, 0.0f)
		, v1(0.0f, 0.0f, 0.0f, 0.0f)
		, v2(0.0f, 0.0f, 0.0f, 0.0f){}
	Triangle(Vec4 _v0, Vec4 _v1, Vec4 _v2) {
		v0 = _v0;
		v1 = _v1;
		v2 = _v2;
	}*/
	Triangle() {}

	//returns C0 - cross product of edge 0
	float edgeFunction(const Vec4& v0, const Vec4& v1, const Vec4& p){
		return (((p.x - v0.x) * (v1.y - v0.y)) - ((v1.x - v0.x) * (p.y - v0.y)));
	}
	

	//tr == top right, bl = bottom left
	void findBounds(const Vec4& v0, const Vec4& v1, const Vec4& v2, Vec4& tr, Vec4& bl, GamesEngineeringBase::Window& canvas) {
		tr.x = min(max(max(v0.x, v1.x), v2.x), canvas.getWidth() - 1);
		tr.y = min(max(max(v0.y, v1.y), v2.y), canvas.getHeight() - 1);
		bl.x = max(min(min(v0.x, v1.x), v2.x), 0);
		bl.y = max(min(min(v0.y, v1.y), v2.y), 0);
	}

	void computeTriangle(const Vec4& v0, const Vec4& v1, const Vec4& v2, Vec4& tr, Vec4& bl
		, GamesEngineeringBase::Window& canvas, float* z_buffer, int bufferWidth, float v0_w, float v1_w, float v2_w) {
		float projArea = edgeFunction(v0, v1, v2);

		for (int y = (int)bl.y; y < (int)tr.y + 1; y++) {
			for (int x = (int)bl.x; x < (int)tr.x + 1; x++) {
				Vec4 p(x + 0.5f, y + 0.5f, 0, 0);

				float alpha = edgeFunction(v1, v2, p);
				float beta = edgeFunction(v2, v0, p);
				float gamma = edgeFunction(v0, v1, p);

				float area = 1.0f / projArea;

				alpha *= area;
				beta *= area;
				gamma *= area;
				//std::cout << alpha << '\t' << beta << '\t' << gamma << std::endl;

				if (alpha > 0 && alpha < 1
					&& beta > 0 && beta < 1
					&& gamma > 0 && gamma < 1) {
					float frag_w = ((alpha * v0_w) + (beta * v1_w) + (gamma * v2_w));
					float zfrag = perspectiveCorrectInterpolateAttribute(float(v0.z), float(v1.z), float(v2.z), v0_w, v1_w, v2_w, alpha, beta, gamma, frag_w);
					Colour frag = perspectiveCorrectInterpolateAttribute(Colour(1.0f, 0, 0, 1), Colour(0, 1.0f, 0, 1), Colour(0, 0, 1.0f, 1), v0_w, v1_w, v2_w, alpha, beta, gamma, frag_w);
					int idx = y * bufferWidth + x;
					if (zfrag < z_buffer[idx]) {
						z_buffer[idx] = zfrag;
						canvas.draw(x, y, frag.r * 255, frag.g * 255, frag.b * 255);
					}
				}
			}
		}
	}

	Vec4 vertToScreenSpace(Vec4& vWorld, Matrix& view, Matrix& proj, GamesEngineeringBase::Window& canvas, float& clip_w) {
		Matrix vp = proj.mul(view);

		Vec4 clip = vp.mul(vWorld);

		clip_w = 1/clip.w;

		clip.divW();
		
		Vec4 pixel;
		pixel.x = ((clip.x + 1) / 2) * canvas.getWidth();
		pixel.y = ((1 - clip.y) / 2) * canvas.getHeight(); //as canvas y increased downward
		pixel.z = clip.z;
		pixel.w = clip.w;
		return pixel;
	}

	Matrix lookAtMatrix(Vec3& from, Vec3& to, Vec3& up) {
		Vec3 dir = (to - from).normalise();
		Vec3 right = (up.Cross(dir)).normalise();
		Vec3 up2 = dir.Cross(right);

		Matrix look;
		look[0] = right.x;
		look[1] = right.y;
		look[2] = right.z;
		
		look[4] = up2.x;
		look[5] = up2.y;
		look[6] = up2.z;

		look[8] = dir.x;
		look[9] = dir.y;
		look[10] = dir.z;

		look[3] = -right.Dot(from);
		look[7] = -up2.Dot(from);
		look[11] = -dir.Dot(from);

		return look;
	}
};

void trianglePipeline() {

}


int main() {
	GamesEngineeringBase::Window canvas;

	canvas.create(1024, 768, "Tile");


	Vec4 tr;
	Vec4 bl;

	Vec4 wv0(0.0f, 0.05f, 1.0f, 1.0f);
	Vec4 wv1(6.0f, -0.03f, 1.0f, 1.0f);
	Vec4 wv2(-3.0f, -0.05f, 1.0f, 1.0f);//z values control distance of points
	Vec4 sv0;
	Vec4 sv1;
	Vec4 sv2;
	float v0_w, v1_w, v2_w;

	Triangle t;
	Matrix projMatrix;

	const float width = (float)canvas.getWidth();
	const float height = (float)canvas.getHeight();
	const float aspect = width / height;
	const float fov = M_PI / 4.0f;
	const float _near = 0.1f;
	const float _far = 1000.0f;
	projMatrix.projMat(fov, aspect, _far, _near);

	// Build view matrix using your lookAtMatrix (ensure its convention matches your math)
	Vec3 from = Vec3(0.0f, 0.0f, 0.0f);    // camera position
	Vec3 to = Vec3(0.0f, 0.0f, 1.0f);    // look direction
	Vec3 up = Vec3(0.0f, 1.0f, 0.0f);
	Matrix view;


	while(true){
		canvas.clear();

		if (canvas.keyPressed('W')) wv0.z += 0.1f;
		if (canvas.keyPressed('S')) wv0.z -= 0.1f;
		if (canvas.keyPressed('A')) {
			wv0.x -= 0.1f; wv1.x -= 0.1f; wv2.x -= 0.1f;
		}
		if (canvas.keyPressed('D')) {
			wv0.x += 0.1f; wv1.x += 0.1f; wv2.x += 0.1f;
		}


		view = t.lookAtMatrix(from, to, up);
		
		sv0 = t.vertToScreenSpace(wv0, view, projMatrix, canvas, v0_w);
		sv1 = t.vertToScreenSpace(wv1, view, projMatrix, canvas, v1_w);
		sv2 = t.vertToScreenSpace(wv2, view, projMatrix, canvas, v2_w);

		t.findBounds(sv0, sv1, sv2, tr, bl, canvas);

		int pixelCount = width * height;
		std::unique_ptr<float[]> z_buffer = std::make_unique<float[]>(pixelCount);
		for (unsigned int i = 0; i < pixelCount; i++)
			z_buffer[i] = 1.0f;

		t.computeTriangle(sv0, sv1, sv2, tr, bl, canvas, z_buffer.get(), width, v0_w, v1_w, v2_w);

		canvas.present();
	}

	return 0;
}
//fix rasterization -- DONE
//build 3d triangle using projection matrix -- DONE
//build look at matrix
//implement simple shading -- Moving bunny