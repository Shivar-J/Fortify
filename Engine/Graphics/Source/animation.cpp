#include "animation.h"

Engine::Graphics::Animation::Animation(std::vector<Keyframe> keyframes, bool loop)
	: m_keyframes(std::move(keyframes)), m_looping(loop)
{
	if (m_keyframes.size() < 2)
		throw std::runtime_error("Animation requires at least two keyframes");

	m_duration = m_keyframes.back().time;
	m_currentTime = 0.0f;
}

void Engine::Graphics::Animation::update(float deltatime)
{
	m_currentTime += deltatime;

	if (m_looping) {
		m_currentTime = std::fmod(m_currentTime, m_duration);
	}
	else {
		if (m_currentTime > m_duration) {
			m_currentTime = m_duration;
		}
	}
}

glm::mat4 Engine::Graphics::Animation::currentTransform()
{
	return interpolate(m_currentTime);
}

void Engine::Graphics::Animation::reset() 
{
	m_currentTime = 0.0f;
}

void Engine::Graphics::Animation::setTime(float time)
{
	m_currentTime = glm::clamp(time, 0.0f, m_duration);
}

void Engine::Graphics::Animation::setLooping(bool loop)
{
	m_looping = loop;
}

float Engine::Graphics::Animation::getDuration()
{
	return m_duration;
}

float Engine::Graphics::Animation::getCurrentTime()
{
	return m_currentTime;
}

glm::mat4 Engine::Graphics::Animation::interpolate(float time)
{
	if (time <= m_keyframes.front().time) return m_keyframes.front().transform;
	if (time >= m_keyframes.back().time) return m_keyframes.back().transform;

	for (int i = 0; i < m_keyframes.size() - 1; i++) {
		const Keyframe& k0 = m_keyframes[i];
		const Keyframe& k1 = m_keyframes[i + 1];

		if (time >= k0.time && time <= k1.time) {
			float factor = (time - k0.time) / (k1.time - k0.time);

			return glm::interpolate(k0.transform, k1.transform, factor);
		}
	}

	return glm::mat4(1.0f);
}

