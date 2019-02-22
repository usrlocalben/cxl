#pragma once

#include <vector>
#define NOMINMAX
#include <Windows.h>

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


class ConsoleCanvas {
public:
	ConsoleCanvas(int width, int height) : d_width(width), d_height(height), d_buf(d_width*d_height) {}

public:
	CHAR_INFO* GetDataPtr() { return d_buf.data(); }
	int d_width;
	int d_height;
	std::vector<CHAR_INFO> d_buf; };


inline ConsoleCanvas Flatten(const ConsoleCanvas& a, const ConsoleCanvas& b, int x, int y) {
	ConsoleCanvas out(a.d_width, a.d_height);
	for (int ry=0; ry<a.d_height; ry++) {
		for (int rx=0; rx<a.d_width; rx++) {
			CHAR_INFO ch;
			if ((ry>=y && ry<y+b.d_height) && (rx>=x && rx<x+b.d_width)) {
				int sx = rx-x;
				int sy = ry-y;
				ch = b.d_buf[sy*b.d_width+sx]; }
			else {
				ch = a.d_buf[ry*a.d_width+rx]; }
			out.d_buf[ry*a.d_width+rx] = ch; }}
	return out; }


inline void WriteXY(ConsoleCanvas& canvas, int x, int y, const std::string& text) {
	if (0 <= y && y < canvas.d_height) {
		for (int xx=0; xx<canvas.d_width; xx++) {
			int ti = xx - x;
			if (0 <= ti && ti < text.length()) {
				canvas.d_buf[y*canvas.d_width + xx].Char.AsciiChar = text[ti]; }}}}


inline void WriteXY(ConsoleCanvas& canvas, int x, int y, const std::string& text, const uint8_t attr) {
	if (0 <= y && y < canvas.d_height) {
		for (int xx=0; xx<canvas.d_width; xx++) {
			int ti = xx - x;
			if (0 <= ti && ti < text.length()) {
				canvas.d_buf[y*canvas.d_width + xx].Char.AsciiChar = text[ti];
				canvas.d_buf[y*canvas.d_width + xx].Attributes = attr; }}}}


inline void Fill(ConsoleCanvas& canvas, uint8_t attr) {
	for (auto& cell : canvas.d_buf) {
		cell.Attributes = attr; }}


}  // namespace rclw
}  // namespace rqdq

