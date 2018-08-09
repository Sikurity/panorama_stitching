#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

int fix_bounds(int im_dim, int given_dim) {
	if (im_dim <= given_dim) {
		given_dim = im_dim - 1;
	}
	if (given_dim < 0) {
		given_dim = 0;
	}
	return given_dim;
}

int wrong_bounds(image im, int x, int y, int c) {
	int i = 0;
	if (im.w < x || x < 0) {
		i++;
	}
	if (im.h < y || y < 0) {
		i++;
	}
	if (im.c < c || c < 0) {
		i++;
	}
	return i;
}

float get_pixel(image im, int x, int y, int c)
{
	x = fix_bounds(im.w, x);
	y = fix_bounds(im.h, y);
	c = fix_bounds(im.c, c);
	int index = ((im.h * im.w) * c) + (im.w * y) + x;
	return *(im.data + index);
}

void set_pixel(image im, int x, int y, int c, float v)
{
	x = fix_bounds(im.w, x);
	y = fix_bounds(im.h, y);
	c = fix_bounds(im.c, c);
	int index = ((im.h * im.w) * c) + (im.w * y) + x;
	*(im.data + index) = v;
	return;
}

image copy_image(image im)
{
	image copy = make_image(im.w, im.h, im.c);
	int i;
	int j;
	int k;
	for (i = 0; i < im.w; i++) {
		for (j = 0; j < im.h; j++) {
			for (k = 0; k < im.c; k++) {
				set_pixel(copy, i, j, k, get_pixel(im, i, j, k));
			}
		}
	}
	return copy;
}

image rgb_to_grayscale(image im)
{
	assert(im.c == 3);
	image gray = make_image(im.w, im.h, 1);
	int i;
	int j;
	for (i = 0; i < im.w; i++) {
		for (j = 0; j < im.h; j++) {
			float im_red = get_pixel(im, i, j, 0);
			float im_green = get_pixel(im, i, j, 1);
			float im_blue = get_pixel(im, i, j, 2);
			float grey_pixel = (0.299 * im_red) + (0.587 * im_green) + (0.114*im_blue);
			set_pixel(gray, i, j, 0, grey_pixel);
		}
	}
	return gray;
}

void shift_image(image im, int c, float v)
{
	assert(im.c <= 3);
	int i;
	int j;
	for (i = 0; i < im.w; i++) {
		for (j = 0; j < im.h; j++) {
			float p = get_pixel(im, i, j, c) + v;
			set_pixel(im, i, j, c, p);
		}
	}
}

void clamp_image(image im)
{
	int i;
	int j;
	int k;
	for (i = 0; i < im.w; i++) {
		for (j = 0; j < im.h; j++) {
			for (k = 0; k < im.c; k++) {
				if (get_pixel(im, i, j, k) < 0) {
					set_pixel(im, i, j, k, 0.0);
				}
				if (get_pixel(im, i, j, k) > 1) {
					set_pixel(im, i, j, k, 1.0);
				}
			}
		}
	}
}


// These might be handy
float three_way_max(float a, float b, float c)
{
	return (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);
}

float three_way_min(float a, float b, float c)
{
	return (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c);
}

void rgb_to_hsv(image im)
{
	int i;
	int j;
	for (i = 0; i < im.w; i++) {
		for (j = 0; j < im.h; j++) {
			float r = get_pixel(im, i, j, 0);
			float g = get_pixel(im, i, j, 1);
			float b = get_pixel(im, i, j, 2);
			float max = three_way_max(r, g, b);
			float min = three_way_min(r, g, b);
			float value = max;
			float c = value - min;
			float sat = 0.0;
			float h_not = 0.0;
			float h = 0.0;
			if (!((r + b + g) == 0)) {
				sat = c / value;
			}
			if (value == r) {
				h_not = (g - b) / c;
			}
			else if (value == g) {
				h_not = (b - r) / c + 2;
			}
			else {
				h_not = (r - g) / c + 4;
			}
			if (h_not < 0) {
				h = h_not / 6 + 1;
			}
			else {
				h = h_not / 6;
			}
			set_pixel(im, i, j, 2, value);
			set_pixel(im, i, j, 1, sat);
			set_pixel(im, i, j, 0, h);
		}
	}
}

void hsv_to_rgb(image im)
{
	int i;
	int j;
	for (i = 0; i < im.w; i++) {
		for (j = 0; j < im.h; j++) {
			float hue = get_pixel(im, i, j, 0);
			float sat = get_pixel(im, i, j, 1);
			float val = get_pixel(im, i, j, 2);
			float r = 0.0;
			float g = 0.0;
			float b = 0.0;
			float h_not = hue * 6.0;
			float c = sat * val;
			float m = val - c;
			if (sat == 0) {
				r = val;
				g = val;
				b = val;
			}
			else if (h_not >= 0 && h_not <= 1) {
				r = val;
				b = m;
				g = h_not * c + b;
			}
			else if (h_not >= 1 && h_not <= 2) {
				h_not = h_not - 2;
				g = val;
				b = m;
				r = -(h_not*c - b);
			}
			else if (h_not >= 2 && h_not <= 3) {
				h_not = h_not - 2;
				g = val;
				r = m;
				b = h_not * c + r;
			}
			else if (h_not >= 3 && h_not <= 4) {
				h_not = h_not - 4;
				b = val;
				r = m;
				g = -(h_not*c - r);
			}
			else if (h_not >= 4 && h_not <= 5) {
				h_not = h_not - 4;
				b = val;
				g = m;
				r = h_not * c + g;
			}
			else {
				h_not = (h_not - 6);
				r = val;
				g = m;
				b = -(h_not*c - g);
			}
			set_pixel(im, i, j, 0, r);
			set_pixel(im, i, j, 1, g);
			set_pixel(im, i, j, 2, b);
		}
	}
}