#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

//#include <Windows.h>

namespace rqdq {
namespace rclw {

enum Color {
	Black,
	Blue,
	Green,
	Cyan,
	Red,
	Magenta,
	Brown,
	White,

	StrongBlack,
	StrongBlue,
	StrongGreen,
	StrongCyan,
	StrongRed,
	StrongMagenta,
	StrongBrown,
	StrongWhite,
};

const char Bright = 0x8;

inline uint8_t MakeAttribute(Color bg, Color fg) {
	return (bg<<4)|fg; }


enum ScanCode {
	A = 0x41,
	B = 0x42,
	C = 0x43,
	D = 0x44,
	E = 0x45,
	F = 0x46,
	G = 0x47,
	H = 0x48,
	I = 0x49,
	J = 0x4A,
	K = 0x4B,
	L = 0x4C,
	M = 0x4D,
	N = 0x4E,
	O = 0x4F,
	P = 0x50,
	Q = 0x51,
	R = 0x52,
	S = 0x53,
	T = 0x54,
	U = 0x55,
	V = 0x56,
	W = 0x57,
	X = 0x58,
   	Y = 0x59,
	Z = 0x5A,
	F1 = 0x70,
	F2 = 0x71,
	F3 = 0x72,
	F4 = 0x73,
	F5 = 0x74,
	F6 = 0x75,
	F7 = 0x76,
	F8 = 0x77,
	F9 = 0x78,
	F10 = 0x79,
	F11 = 0x7a,
	F12 = 0x7b
};


enum BoxChar {
	lowerLeft = 0xc0,
	lowerRight = 0xd9,
	upperLeft = 0xda,
	upperRight = 0xbf,
	horizontalLine = 0xc4,
	verticalLine = 0xb3,
	leftTee = 0xb4,
	rightTee = 0xc3 };


class Console {
	// lifetime
private:
	Console(void*, void*);
	~Console();
	Console& operator=(const Console&) = delete;
	Console(const Console&) = delete;
public:
	static Console& GetInstance();

	// size, cursor, buffer management
	void SetDimensions(int width, int height);
	std::pair<int, int> GetDimensions();
	std::pair<int, int> GetCursorPosition();
	void MapKey(uint16_t scanCode, char ch);
	void UnmapKey(uint16_t scanCode);

	// drawing api
	Console& Position(int x, int y) { d_x = x; d_y = y; return *this; }
	Console& LeftEdge(int x) { d_leftEdge = x; return *this; }
	Console& CR() { d_x = d_leftEdge; d_y++; return *this; }
	Console& ShowCursor();
	Console& Write(const std::wstring& text);
	Console& Write(const std::string& text);
	Console& Write(const std::wstring& text, char attr);
	Console& Write(const std::string& text, char attr);
	Console& Clear();

	Console& PushPosition() {
		d_positionStack.emplace_back(d_x, d_y);
		return *this; }
	Console& PopPosition() {
		std::tie(d_x, d_y) = d_positionStack.back();
		d_positionStack.pop_back();
		return *this; }
	Console& AdvanceCursor(int n) {
		d_y += (d_x + n) / d_width;
		d_x = (d_x + n) % d_width;
		return *this; }

private:
	std::vector<std::pair<int, int>> d_positionStack;
	int d_x = 0;
	int d_y = 0;
	int d_height = 0;
	int d_width = 0;
	int d_leftEdge = 0;
	void* d_stdout = nullptr;
	void* d_stdin = nullptr;
	std::unordered_map<uint16_t, int> d_remap;
	std::vector<bool> d_activeKeys; };


}  // close package namespace
}  // close enterprise namespace
