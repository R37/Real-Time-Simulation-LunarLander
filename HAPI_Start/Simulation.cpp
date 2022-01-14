#include "Simulation.h"
//#include <HAPI_lib.h>
#include<time.h>
#include<vector>
#include "particles.h";

//using namespace HAPISPACE;

void Rotate(float& posX, float& posY, float AposX, float AposY, float Angle)
{
	float TPosX;
	float TPosY;
	float RPosX;
	float RPosY;
	float FPosX;
	float FPosY;

	float sinAngle;
	float cosAngle;

	sinAngle = sin(Angle);
	cosAngle = cos(Angle);

	TPosX = posX - AposX;
	TPosY = posY - AposY;

	RPosX = (TPosX * cosAngle) - (TPosY * sinAngle);
	RPosY = (TPosX * sinAngle) + (TPosY * cosAngle);

	FPosX = RPosX + AposX;
	FPosY = RPosY + AposY;

	posX = FPosX;
	posY = FPosY;
}

void DrawLine(BYTE* screen, int screenwidth, int screenheight, int startX, int startY, int endX, int endY, int Red = 0, int Green = 255, int Blue = 0)
{
	int lengthX = endX - startX;
	int lengthY = endY - startY;
	int length = sqrt((lengthX * lengthX) + (lengthY * lengthY));
	float calcX = startX;
	float calcY = startY;

	for (float i = 0; i < 1; i += 1.0f / length / 2)
	{
		calcX = (1 - i) * startX + i * endX;
		calcY = (1 - i) * startY + i * endY;

		int drawX = calcX;
		int drawY = calcY;
		if (drawX > 0 && drawX < screenwidth && drawY > 0 && drawY < screenheight)
		{
			screen[drawY * screenwidth * 4 + drawX * 4] = Red; //Red
			screen[drawY * screenwidth * 4 + drawX * 4 + 1] = Green; //Green
			screen[drawY * screenwidth * 4 + drawX * 4 + 2] = Blue; //Blue
		}

	}

}

void Simulation::TriggerParticles()
{
	for (int z = 0; z < ParticlesVec.size(); z++)
	{
		ParticlesVec[z]->Setup(posx, posy);
		ParticlesVec[z]->SetIsAlive(true);
	}
}

void Simulation::Run()
{
	//Particles

	float TotalParticles = 2000;
	for (int x = 0; x <= TotalParticles; x++)
	{
		Particles* newParticle = new Particles();
		ParticlesVec.push_back(newParticle);
	}


	HAPI_TColour textcol{ HAPI_TColour::RED };
	HAPI_TColour outcol{ HAPI_TColour::RED };


	int width{ 1280 };
	int height{ 720 };

	if (!HAPI.Initialise(width, height))
		return;
	HAPI.SetShowFPS(true);
	BYTE* screen = HAPI.GetScreenPointer();

	


	//Initilise Keyboard Input
	const HAPI_TKeyboardData& KeyData = HAPI.GetKeyboardData();

	while (HAPI.Update())
	{
		//Delta Time
		Time = clock();
		DeltaTime = Time - OldTime;
		OldTime = Time;


		//Clear Screen
		memset(screen, 0, (size_t)width * height * 4);
		//UI
		HAPI.RenderText(25, 50, textcol, outcol, 1, "Velocity Y: " + std::to_string(Vy), 32);
		HAPI.RenderText(25, 82, textcol, outcol, 1, "Velocity X: " + std::to_string(Vx), 32);
		HAPI.RenderText(25, 112, textcol, outcol, 1, "PosX: " + std::to_string(posx), 32);
		HAPI.RenderText(25, 144, textcol, outcol, 1, "PosY: " + std::to_string(posy), 32);
		HAPI.RenderText(25, 176, textcol, outcol, 1, "Angle: " + std::to_string(Angle), 32);
		//Draw Triangle(Lander)
		float TopX = posx;
		float TopY = posy - 20;
		float LeftX = posx - 20;
		float LeftY = posy + 20;
		float RightX = posx + 20;
		float RightY = posy + 20;

		//Controls
		if (If_Landed == false)
		{
			if (KeyData.scanCode['A'])
			{
				Angle -= 0.005;
			}
			else if (KeyData.scanCode['D'])
			{
				Angle += 0.005;
			}
			else if (KeyData.scanCode['W'])
			{
				Vx -= ThrustX / 10000;
				Vy -= ThrustY / 10000;
			}
		}
		if (KeyData.scanCode['R'])
		{
			posx = 200;
			posy = 200;
			Vx = 0;
			Vy = 0;
			Gravity = 0.001;
			If_Landed = false;
			Crashed = false;
			Angle = 0;
			for (int z = 0; z < ParticlesVec.size(); z++)
			{;
				ParticlesVec[z]->SetIsAlive(false);
			}
		}

		//Rotate Lander
		Rotate(TopX, TopY, posx, posy, Angle);
		Rotate(LeftX, LeftY, posx, posy, Angle);
		Rotate(RightX, RightY, posx, posy, Angle);

		//Thrust
		ThrustY = posy - TopY;
		ThrustX = posx - TopX;

		//Draw Lander
		DrawLine(screen, width, height, LeftX, LeftY, RightX, RightY, 0, 0, 255);
		DrawLine(screen, width, height, LeftX, LeftY, TopX, TopY, 0, 0, 255);
		DrawLine(screen, width, height, RightX, RightY, TopX, TopY, 0, 0, 255);

		//Collision with bottom of screen
		if (posy > 700 && Vy > 0.4 && If_Landed == false)
		{
			If_Landed = true;
			Vy = 0;
			Vx = 0;
			Gravity = 0;
			Crashed = true;
			//Angle = 0;
			TriggerParticles();
		}
		else if (posy > 700 && Vy < 0.4 && If_Landed == false)
		{
			if (Angle > -0.2 && Angle < 0.2)
			{
				If_Landed = true;
				Vy = 0;
				Vx = 0;
				Gravity = 0;
				Crashed = false;
				//Angle = 0;
			}
			else
			{
				If_Landed = true;
				Vy = 0;
				Vx = 0;
				Gravity = 0;
				Crashed = true;
				//Angle = 0;
				TriggerParticles();
			}

		}
		if (posx <= 10)
		{
			If_Landed = true;
			Vy = 0;
			Vx = 0;
			Gravity = 0;
			Crashed = true;
			//Angle = 0;
			TriggerParticles();
		}
		else if (posx >= 1270)
		{
			If_Landed = true;
			Vy = 0;
			Vx = 0;
			Gravity = 0;
			Crashed = true;
			//Angle = 0;
			TriggerParticles();
		}
		//Update all entitys
		for (Particles* p : ParticlesVec)
			p->Update(screen, width, height);


		if (Crashed == true)
		{
			HAPI.RenderText(500, 300, textcol, outcol, 1, "You Crashed!", 64);
		}
		else if (If_Landed && Crashed == false)
		{
			HAPI.RenderText(500, 300, textcol, outcol, 1, "You Landed Succesfully!", 64);
		}


		//Velocity
		Vy += Gravity;
		posx += Vx * DeltaTime;
		posy += Vy * DeltaTime;

		//Screen Borders
		//bottom
		DrawLine(screen, width, height, 0, 719, 1280, 719);
		//Left
		DrawLine(screen, width, height, 1, 0, 1, 720);
		//Right
		DrawLine(screen, width, height, 1279, 0, 1279, 720);

	}
	
}

