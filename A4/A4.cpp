#include <glm/ext.hpp>

#include "A4.hpp"

void A4_Render(
		// What to render  
		SceneNode * root,

		// Image to write to, set to a given width and height  
		Image & image,

		// Viewing parameters  
		const glm::vec3 & eye,
		const glm::vec3 & view,
		const glm::vec3 & up,
		double fovy,

		// Lighting parameters  
		const glm::vec3 & ambient,
		const std::list<Light *> & lights)
{
	// Construct the orthonormal coordinate / camera frame {u,v,w} from eye,view,up
	
	// We choose w to be opposite the view vector, and all rays to have direction -w
	glm::vec3 w{-1 * glm::normalize(view)};
	glm::vec3 u{glm::normalize(glm::cross(up,w))};
	// This chooses 'v' so that it is the closest one to 'up' that is orthogonal to w
	glm::vec3 v{glm::cross(w,u)};

	

	std::cout << "Calling A4_Render(\n" <<
		"\t" << *root <<
		"\t" << "Image(width:" << image.width() << ", height:" << image.height() << ")\n"
		"\t" << "eye:  " << glm::to_string(eye) << std::endl <<
		"\t" << "view: " << glm::to_string(view) << std::endl <<
		"\t" << "up:   " << glm::to_string(up) << std::endl <<
		"\t" << "fovy: " << fovy << std::endl <<
		"\t" << "ambient: " << glm::to_string(ambient) << std::endl <<
		"\t" << "lights{" << std::endl;

	for(const Light * light : lights) {
		std::cout << "\t\t" <<  *light << std::endl;
	}
	std::cout << "\t}" << std::endl;
	std:: cout <<")" << std::endl;

	size_t h = image.height();
	size_t w = image.width();

	for (uint y = 0; y < h; ++y) {
		for (uint x = 0; x < w; ++x) {
			// Red: 
			image(x, y, 0) = (double)1.0;
			// Green: 
			image(x, y, 1) = (double)1.0;
			// Blue: 
			image(x, y, 2) = (double)1.0;
		}
	}

}
