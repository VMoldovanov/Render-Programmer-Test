#pragma once

////////////////////////////////////////////////////////////////////////////////
// Код, с небольшими правками, был взят с нижеуказанного ресурса: 
// https://github.com/oguz81/ArcballCamera
////////////////////////////////////////////////////////////////////////////////
class ArcballCamera final
{
   struct Quaternion
   {
      float cosine;     //cosine of half the rotation angle
      glm::vec3 axis;   //unit vector scaled by sine of half the angle
   };

public:
   ArcballCamera(int widthScreen, int heightScreen)
      : width(widthScreen), height(heightScreen), radius(1.0f), flag(false)
   {}

   void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
   {
      if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
      {
         double startXPos, startYPos; //screen coordinates when mouse clicks.
         glfwGetCursorPos(window, &startXPos, &startYPos);
         startPos.x = (((float)startXPos - (width / 2.f)) / (width / 2.f)) * radius; //convert to NDC, then assign to startPos.
         startPos.y = (((height / 2.f) - (float)startYPos) / (height / 2.f)) * radius;// ..same for y coordinate.
         startPos.z = z_axis(startPos.x, startPos.y, radius);
         flag = true;
      }
      else if (action == GLFW_RELEASE)
      {
         replace();
         flag = false;
      }
   }

   void cursorPosCallback(GLFWwindow* window, double x, double y)
   {
      if (flag == true)
      {
         //Get the screen coordinates when mouse clicks.
         currentPos.x = (((float)x - (width / 2)) / (width / 2)) * radius;
         currentPos.y = (((height / 2) - (float)y) / (height / 2)) * radius;
         currentPos.z = z_axis(currentPos.x, currentPos.y, radius);
         rotation();
      }
   }

   float GetAngle()
   {
      return angle;
   }

   glm::vec3 GetRotationalAxis()
   {
      return rotationalAxis;
   }

private:
   float z_axis(float x, float y, float radius)
   {
      float z = 0;
      if (sqrt((x * x) + (y * y)) <= radius) z = (float)sqrt((radius * radius) - (x * x) - (y * y));
      return z;
   }

   glm::vec3 getUnitVector(glm::vec3 vectr)
   {
      float magnitude1;
      glm::vec3 unitVector;
      magnitude1 = (vectr.x * vectr.x) + (vectr.y * vectr.y) + (vectr.z * vectr.z);
      magnitude1 = sqrt(magnitude1);
      if (magnitude1 == 0) {
         unitVector.x = 0;
         unitVector.y = 0;
         unitVector.z = 0;
      }
      else {
         unitVector.x = vectr.x / magnitude1; //if magnitude 0, then this func give 'nan' error.
         unitVector.y = vectr.y / magnitude1;
         unitVector.z = vectr.z / magnitude1;
      }
      return unitVector;
   }

   float dotProduct()
   {
      float result = (startPosUnitVector.x * currentPosUnitVector.x) + (startPosUnitVector.y * currentPosUnitVector.y) + (startPosUnitVector.z * currentPosUnitVector.z);
      return result;
   }

   void rotation()
   {
      startPosUnitVector = getUnitVector(startPos);
      currentPosUnitVector = getUnitVector(currentPos);
      currentQuaternion.axis = glm::cross(startPos, currentPos);
      currentQuaternion.axis = getUnitVector(currentQuaternion.axis);

      cosValue = dotProduct(); //q0 is cosine of the angle here.
      if (cosValue > 1) cosValue = 1.f; // when dot product gives '1' as result, it doesn't equal to 1 actually. It equals to just like 1.00000000001 . 
      theta = (acos(cosValue) * 180.f / glm::pi<float>()); //theta is the angle now.
      currentQuaternion.cosine = cos((theta / 2.f) * glm::pi<float>() / 180.f); //currentQuaternion.cosine is cos of half the angle now.

      currentQuaternion.axis.x = currentQuaternion.axis.x * sin((theta / 2.f) * glm::pi<float>() / 180.f);
      currentQuaternion.axis.y = currentQuaternion.axis.y * sin((theta / 2.f) * glm::pi<float>() / 180.f);
      currentQuaternion.axis.z = currentQuaternion.axis.z * sin((theta / 2.f) * glm::pi<float>() / 180.f);

      cosValue_2 = (currentQuaternion.cosine * lastQuaternion.cosine)
         - glm::dot(currentQuaternion.axis, lastQuaternion.axis);


      glm::vec3 temporaryVector;

      temporaryVector = glm::cross(currentQuaternion.axis, lastQuaternion.axis);


      rotationalAxis_2.x = (currentQuaternion.cosine * lastQuaternion.axis.x) +
         (lastQuaternion.cosine * currentQuaternion.axis.x) +
         temporaryVector.x;

      rotationalAxis_2.y = (currentQuaternion.cosine * lastQuaternion.axis.y) +
         (lastQuaternion.cosine * currentQuaternion.axis.y) +
         temporaryVector.y;

      rotationalAxis_2.z = (currentQuaternion.cosine * lastQuaternion.axis.z) +
         (lastQuaternion.cosine * currentQuaternion.axis.z) +
         temporaryVector.z;

      angle = (acos(cosValue_2) * 180.f / glm::pi<float>()) * 2.f;

      rotationalAxis.x = rotationalAxis_2.x / sin((angle / 2.f) * glm::pi<float>() / 180.f);
      rotationalAxis.y = rotationalAxis_2.y / sin((angle / 2.f) * glm::pi<float>() / 180.f);
      rotationalAxis.z = rotationalAxis_2.z / sin((angle / 2.f) * glm::pi<float>() / 180.f);
   }

   void replace()
   {
      lastQuaternion.cosine = cosValue_2;
      lastQuaternion.axis = rotationalAxis_2;
   }

private:
   glm::vec3 position = glm::vec3(0.0f, 0.0f, -3.0f);
   glm::vec3 startPos = glm::vec3(0.0f, 0.0f, 0.0f);
   glm::vec3 currentPos = startPos;
   glm::vec3 startPosUnitVector = glm::vec3(0.0f, 0.0f, 0.0f);
   glm::vec3 currentPosUnitVector = glm::vec3(0.0f, 0.0f, 0.0f);

   Quaternion lastQuaternion = { 0.0f, glm::vec3(0.0f, 1.0f, 0.0f) };
   Quaternion currentQuaternion = lastQuaternion;

   float cosValue = 0.0f, cosValue_2 = 0.0f;
   float theta = 0.0f;
   float angle = 180.0f;
   glm::vec3 rotationalAxis = glm::vec3(0.0f, 1.0f, 0.0f);
   glm::vec3 rotationalAxis_2 = rotationalAxis;

   int width, height;
   float radius;
   bool flag;
};