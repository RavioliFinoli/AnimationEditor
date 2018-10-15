#include "InputHandler.h"



InputHandler::InputHandler()
{
}


InputHandler::~InputHandler()
{
}

void InputHandler::SetWPressed(bool state)
{
	W_isPressed = state;
}

void InputHandler::SetAPressed(bool state)
{
	A_isPressed = state;
}

void InputHandler::SetSPressed(bool state)
{
	S_isPressed = state;
}

void InputHandler::SetDPressed(bool state)
{
	D_isPressed = state;
}

bool InputHandler::GetWState()
{
	return W_isPressed;
}

bool InputHandler::GetAState()
{
	return A_isPressed;
}

bool InputHandler::GetSState()
{
	return S_isPressed;
}

bool InputHandler::GetDState()
{
	return D_isPressed;
}

InputHandler gInputHandler;
