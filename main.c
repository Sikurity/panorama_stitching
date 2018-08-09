#define _CRT_SECURE_NO_WARNINGS

#include <math.h>
#include <string.h>
#include "image.h"
#include "test.h"
#include "args.h"

void draw_corners() {
	image im = load_image("data/Rainier1.png");
	detect_and_draw_corners(im, 2, 50, 3);
	save_image(im, "corners");

	free_image(im);
}

void draw_matched() {
	image im1 = load_image("data/Rainier1.png");
	image im2 = load_image("data/Rainier2.png");
	image matched = find_and_draw_matches(im1, im2, 2, 50, 3);
	save_image(matched, "matches");

	free_image(im1);
	free_image(im2);
	free_image(matched);
}

void easy_panorama() {
	image im1 = load_image("data/Rainier1.png");
	image im2 = load_image("data/Rainier2.png");
	image pan = panorama_image(im1, im2, 2, 50, 3, 2, 10000, 30);
	save_image(pan, "easy_panorama");

	free_image(im1);
	free_image(im2);
	free_image(pan);
}

void rainier_panorama() {
	image im1 = load_image("data/Rainier1.png");
	image im2 = load_image("data/Rainier2.png");
	image im3 = load_image("data/Rainier3.png");
	image im4 = load_image("data/Rainier4.png");
	image im5 = load_image("data/Rainier5.png");
	image im6 = load_image("data/Rainier6.png");
	
	image pan1 = panorama_image(im1, im2, 2, 5, 3, 2, 10000, 30);
	save_image(pan1, "rainier_panorama_1");
	image pan2 = panorama_image(pan1, im5, 2, 5, 3, 2, 10000, 30);
	save_image(pan2, "rainier_panorama_2");
	image pan3 = panorama_image(pan2, im6, 2, 5, 3, 2, 10000, 30);
	save_image(pan3, "rainier_panorama_3");
	image pan4 = panorama_image(pan3, im3, 2, 5, 3, 2, 10000, 30);
	save_image(pan4, "rainier_panorama_4");
	image pan5 = panorama_image(pan4, im4, 2, 5, 3, 2, 10000, 30);
	save_image(pan5, "rainier_panorama_5");

	free_image(im1);
	free_image(im2);
	free_image(im3);
	free_image(im4);
	free_image(im5);
	free_image(im6);
	free_image(pan1);
	free_image(pan2);
	free_image(pan3);
	free_image(pan4);
	free_image(pan5);
}

void field_panorama() {
	image im1 = load_image("data/field1.jpg");
	image im2 = load_image("data/field2.jpg");
	image im3 = load_image("data/field3.jpg");
	image im4 = load_image("data/field4.jpg");
	image im5 = load_image("data/field5.jpg");
	image im6 = load_image("data/field6.jpg");
	image im7 = load_image("data/field7.jpg");
	image im8 = load_image("data/field8.jpg");

	im1 = cylindrical_project(im1, 1200);
	im2 = cylindrical_project(im2, 1200);
	im3 = cylindrical_project(im3, 1200);
	im4 = cylindrical_project(im4, 1200);
	im5 = cylindrical_project(im5, 1200);
	im6 = cylindrical_project(im6, 1200);
	im7 = cylindrical_project(im7, 1200);
	im8 = cylindrical_project(im8, 1200);
	
	save_image(im1, "cylindrical_projection");

	image pan1 = panorama_image(im5, im6, 2, 2, 3, 3, 50000, 30);
	save_image(pan1, "field_panorama_1");
	image pan2 = panorama_image(pan1, im7, 2, 2, 3, 3, 50000, 30);
	save_image(pan2, "field_panorama_2");
	image pan3 = panorama_image(pan2, im8, 2, 2, 3, 3, 50000, 30);
	save_image(pan3, "field_panorama_3");
	image pan4 = panorama_image(pan3, im4, 2, 2, 3, 3, 50000, 30);
	save_image(pan4, "field_panorama_4");
	image pan5 = panorama_image(pan4, im3, 2, 2, 3, 3, 50000, 30);
	save_image(pan5, "field_panorama_5");

	free_image(im1);
	free_image(im2);
	free_image(im3);
	free_image(im4);
	free_image(im5);
	free_image(im6);
	free_image(im7);
	free_image(im8);
	free_image(pan1);
	free_image(pan2);
	free_image(pan3);
	free_image(pan4);
	free_image(pan5);
}

int main(int argc, char **argv)
{
	int mode, flag = 1;

	while (flag) {
		printf("What do you want? Exit = 0\n");
		scanf("%d", &mode);
		switch (mode) {
		case 0:
			flag = 0;
			break;
		case 1:
			printf("Gray Scale Image...\n");

			image im = load_image("data/dog.jpg");
			image g = rgb_to_grayscale(im);
			save_image(g, "out");

			free_image(im);
			free_image(g);
			break;
		case 2:
			draw_corners();
			break;
		case 3:
			draw_matched();
			break;
		case 4:
			easy_panorama();
			break;
		case 5:
			rainier_panorama();
			break;
		case 6:
			field_panorama();
			break;
		case 7:
			printf("Run Test...\n");
			run_tests();
			break;
		default:
			printf("Mode %d Not Exist\n", mode);
		}
	}

    return 0;
}
