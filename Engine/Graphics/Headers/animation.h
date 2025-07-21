#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>
#include <stdexcept>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

struct Keyframe {
	float time;
	glm::mat4 transform;
};

namespace Engine::Graphics {
	class Animation {
	public:
		Animation() = default;

		explicit Animation(std::vector<Keyframe> keyframes, bool loop = true);

		void update(float deltatime);
		glm::mat4 currentTransform();

		void reset();
		void setTime(float time);
		void setLooping(bool loop);

		float getDuration();
		float getCurrentTime();

	private:
		glm::mat4 interpolate(float time);

		std::vector<Keyframe> m_keyframes;
		float m_duration = 0.0f;
		float m_currentTime = 0.0f;
		bool m_looping = true;
	};
}

#endif