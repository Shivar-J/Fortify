#ifndef CAMERA_H
#define CAMERA_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>

enum class Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

const float SPEED = 1.0f;
const float SENSITIVITY = 0.1f;

namespace Engine::Core {
	class Camera {
	public:
		glm::vec3 Position;
		glm::quat Orientation;
		float RightAngle;
		float UpAngle;
		
		float MovementSpeed;
		float MouseSensitivity;

		float FOV = 45.0f;
		float NearClip = 0.1f;
		float FarClip = 1000.0f;
		float AspectRatio = 16.0f / 9.0f;

		Camera(glm::vec3 position) : MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY) {
			Position = position;
			Orientation = glm::quat(0, 0, 0, -1);
			RightAngle = 0.0f;
			UpAngle = 0.0f;
			updateCameraVectors();
		}

		Camera(glm::vec3 position, glm::quat orientation) : MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY) {
			Position = position;
			Orientation = orientation;
			RightAngle = 0.0f;
			UpAngle = 0.0f;
			updateCameraVectors();
		}

		Camera(float posX, float posY, float posZ) : MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY) {
			Position = glm::vec3(posX, posY, posZ);
			Orientation = glm::quat(0, 0, 0, -1);
			RightAngle = 0.0f;
			UpAngle = 0.0f;
			updateCameraVectors();
		}

	public:
		glm::mat4 GetViewMatrix() {
			glm::quat reverseOrientation = glm::conjugate(Orientation);
			glm::mat4 rotation = glm::mat4_cast(reverseOrientation);
			glm::mat4 translation = glm::translate(glm::mat4(1.0), -Position);

			return rotation * translation;
		}

		glm::mat4 GetProjectionMatrix() {
			glm::mat4 proj = glm::perspective(glm::radians(FOV), AspectRatio, NearClip, FarClip);
			proj[1][1] *= -1;
			return proj;
		}

		void processKeyboard(Camera_Movement direction, float deltaTime) {
			float velocity = MovementSpeed * deltaTime;

			glm::quat qF = Orientation * glm::quat(0, 0, 0, -1) * glm::conjugate(Orientation);
			glm::vec3 Front = { qF.x, qF.y, qF.z };
			glm::vec3 Right = glm::normalize(glm::cross(Front, glm::vec3(0, 1, 0)));
			glm::vec3 Up = glm::normalize(glm::cross(Right, Front));

			if (direction == Camera_Movement::FORWARD)
				Position += Front * velocity;
			if (direction == Camera_Movement::BACKWARD)
				Position -= Front * velocity;
			if (direction == Camera_Movement::LEFT)
				Position -= Right * velocity;
			if (direction == Camera_Movement::RIGHT)
				Position += Right * velocity;
			if (direction == Camera_Movement::UP)
				Position += Up * velocity;
			if (direction == Camera_Movement::DOWN)
				Position -= Up * velocity;

			updateCameraVectors();
		}

		void processMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
			xoffset *= MouseSensitivity;
			yoffset *= MouseSensitivity;

			RightAngle += xoffset;
			UpAngle += yoffset;

			if (UpAngle >= 89.0f)
				UpAngle = 89.0f;
			if (UpAngle <= -89.0f)
				UpAngle = -89.0f;

			updateCameraVectors();
		}

		void updateCameraVectors() {
			glm::quat aroundY = glm::angleAxis(glm::radians(-RightAngle), glm::vec3(0, 1, 0));
			glm::quat aroundX = glm::angleAxis(glm::radians(UpAngle), glm::vec3(1, 0, 0));

			Orientation = aroundY * aroundX;

			Orientation = glm::normalize(Orientation);
		}
	};
}

#endif