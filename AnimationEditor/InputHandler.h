#pragma once

class InputHandler
{
public:
	InputHandler();
	~InputHandler();

	void SetWPressed(bool state = true);
	void SetAPressed(bool state = true);
	void SetSPressed(bool state = true);
	void SetDPressed(bool state = true);

	bool GetWState();
	bool GetAState();
	bool GetSState();
	bool GetDState();
private:
	bool W_isPressed = false;
	bool A_isPressed = false;
	bool S_isPressed = false;
	bool D_isPressed = false;
};

extern InputHandler gInputHandler;

