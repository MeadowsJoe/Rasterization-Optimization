#include <iostream>

#include "Maths.h"
#include "GamesEngineeringBase.h"
#include "GEMLoader.h"


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

struct Camera {
	Vec3 from;
	Vec3 to;
	Vec3 up;
	Matrix view;
	Matrix projection;
};

struct TriangleInputs {
	Vec4 wv0, wv1, wv2;
	Vec3 N0, N1, N2;
};

struct ScreenInfo {
	float width;
	float height;
	float* z_buffer;
	Vec4 topRight;
	Vec4 bottomLeft;
};

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
	float edgeFunction(const Vec4& v0, const Vec4& v1, const Vec4& p) {
		return (((p.x - v0.x) * (v1.y - v0.y)) - ((v1.x - v0.x) * (p.y - v0.y)));
	}

	//tr == top right, bl = bottom left
	void findBounds(const Vec4& v0, const Vec4& v1, const Vec4& v2, Vec4& tr, Vec4& bl, GamesEngineeringBase::Window& canvas) {
		tr.x = min(max(max(v0.x, v1.x), v2.x), canvas.getWidth() - 1);
		tr.y = min(max(max(v0.y, v1.y), v2.y), canvas.getHeight() - 1);
		bl.x = max(min(min(v0.x, v1.x), v2.x), 0);
		bl.y = max(min(min(v0.y, v1.y), v2.y), 0);
	}

	Colour ambientLightShading(Vec3& N0, Vec3& N1, Vec3& N2, float v0_w, float v1_w, float v2_w, float alpha, float beta, float gamma, float frag_w) {
		Vec3 omega_i = Vec3(1, 1, 0).normalise();
		Vec3 N = perspectiveCorrectInterpolateAttribute(N0, N1, N2, v0_w, v1_w, v2_w, alpha, beta, gamma, frag_w).normalise();
		Colour rho(0.98f, 0.6f, 0.78f, 1.0f);
		Colour L(1.0f, 1.0f, 1.0f, 1.0f);
		Colour ambient(0.6f, 0.6f, 0.6f, 1.0f);
		return ((rho / M_PI) * (L * max(Dot(omega_i, N), 0.0f) + ambient));
	}

	void computeTriangle(const Vec4& v0, const Vec4& v1, const Vec4& v2, Vec3& n0, Vec3& n1, Vec3& n2, Vec4& tr, Vec4& bl
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
					//Colour frag = perspectiveCorrectInterpolateAttribute(Colour(1.0f, 0, 0, 1), Colour(0, 1.0f, 0, 1), Colour(0, 0, 1.0f, 1), v0_w, v1_w, v2_w, alpha, beta, gamma, frag_w);
					Colour frag = ambientLightShading(n0, n1, n2, v0_w, v1_w, v2_w, alpha, beta, gamma, frag_w);
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
		Matrix viewProj = proj.mul(view);

		Vec4 clip = viewProj.mul(vWorld);

		clip_w = 1/clip.w;

		clip.divW();

		Vec4 pixel;
		pixel.x = ((clip.x + 1) / 2) * canvas.getWidth();
		pixel.y = ((1 - clip.y) / 2) * canvas.getHeight(); //as canvas y increased downward
		pixel.z = clip.z;
		pixel.w = clip.w;
		return pixel;
	}

};

void trianglePipeline(
	GamesEngineeringBase::Window& canvas,
	Camera& cam,
	Triangle& t,
	TriangleInputs& in,
	ScreenInfo& screen) {

	float v0_w; float v1_w; float v2_w;

	Vec4 sv0 = t.vertToScreenSpace(in.wv0, cam.view, cam.projection, canvas, v0_w);
	Vec4 sv1 = t.vertToScreenSpace(in.wv1, cam.view, cam.projection, canvas, v1_w);
	Vec4 sv2 = t.vertToScreenSpace(in.wv2, cam.view, cam.projection, canvas, v2_w);

	t.findBounds(sv0, sv1, sv2, screen.topRight, screen.bottomLeft, canvas);

	t.computeTriangle(sv0, sv1, sv2
		, in.N0, in.N1, in.N2
		, screen.topRight, screen.bottomLeft
		, canvas, screen.z_buffer, screen.width
		, v0_w, v1_w, v2_w);
}


int main() {
	GamesEngineeringBase::Window canvas;

	canvas.create(1024, 768, "Tile");

	GamesEngineeringBase::Timer tim;

	float time = 0;
	const float width = (float)canvas.getWidth();
	const float height = (float)canvas.getHeight();
	const float aspect = width / height;
	const float fov = M_PI / 2.0f;
	const float _near = 0.1f;
	const float _far = 1000.0f;
	float r = 0.5f;
	float angle = 0.0f;
	float angleSpeed = 0.005f;

	Camera cam;
	// Build view matrix using your lookAt() (ensure its convention matches your math)
	cam.from = Vec3(0.0f, 0.08f, 0.0f);    // camera position //from
	cam.to = Vec3(0.0f, 0.05f, 0.0f);    // look direction //to
	cam.up = Vec3(0.0f, 1.0f, 0.0f);
	cam.projection.projMat(fov, aspect, _far, _near);

	Vec4 tr, bl;
	Triangle t;
	
	std::vector<GEMLoader::GEMMesh> gemmeshes;
	GEMLoader::GEMModelLoader loader;

	loader.load("Resources/bunny.gem", gemmeshes);
	std::vector<Vec3> vertexList;
	std::vector<Vec3> normalList;
	for (int i = 0; i < gemmeshes.size(); i++) {
		for (int j = 0; j < gemmeshes[i].indices.size(); j++) {
			GEMLoader::GEMVec3 vec;
			GEMLoader::GEMVec3 nor;
			int index = gemmeshes[i].indices[j];
			vec = gemmeshes[i].verticesStatic[index].position;
			nor = gemmeshes[i].verticesStatic[index].normal;
			vertexList.push_back(Vec3(vec.x, vec.y, vec.z));
			normalList.push_back(Vec3(nor.x, nor.y, nor.z));
		}
	}

	while (true) {
		canvas.clear();

		if (canvas.keyPressed(VK_UP)) r += 0.05;
		if (canvas.keyPressed(VK_DOWN)) r -= 0.05;

		float dt = tim.dt();
		time += dt;

		cam.from = Vec3(r * cos(time), cam.from.y, r * sinf(time));
		cam.view = Matrix::lookAt(cam.from, cam.to, cam.up); 
		//this lookat funciton is static, meaning it belongs to the class itself and not the instance, so dont need to create an instance to call it.

		int pixelCount = width * height;
		std::unique_ptr<float[]> z_buffer = std::make_unique<float[]>(pixelCount);
		for (unsigned int i = 0; i < pixelCount; i++)
			z_buffer[i] = 1.0f;

		ScreenInfo screen{ width, height, z_buffer.get(), tr, bl };

		for (unsigned int i = 0; i < vertexList.size()/3; i++) {
			TriangleInputs tri{
			Vec4 (vertexList[3 * i + 0]),
			Vec4 (vertexList[3 * i + 1]),
			Vec4 (vertexList[3 * i + 2]),
			normalList[3 * i + 0],
			normalList[3 * i + 1],
			normalList[3 * i + 2]
			};
			trianglePipeline(canvas, cam, t, tri, screen);
		}

		canvas.present();
	}

	return 0;
}