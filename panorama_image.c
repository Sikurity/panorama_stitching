#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <float.h>
#include "image.h"
#include "matrix.h"

// Comparator for matches
// const void *a, *b: pointers to the matches to compare.
// returns: result of comparison, 0 if same, 1 if a > b, -1 if a < b.
int match_compare(const void *a, const void *b)
{
	match *ra = (match *)a;
	match *rb = (match *)b;
	if (ra->distance < rb->distance) return -1;
	else if (ra->distance > rb->distance) return  1;
	else return 0;
}

// Helper function to create 2d points.
// float x, y: coordinates of point.
// returns: the point. 
point make_point(float x, float y)
{
	point p;
	p.x = x; p.y = y;
	return p;
}

// Place two images side by side on canvas, for drawing matching pixels.
// image a, b: images to place.
// returns: image with both a and b side-by-side.
image both_images(image a, image b)
{
	image both = make_image(a.w + b.w, a.h > b.h ? a.h : b.h, a.c > b.c ? a.c : b.c);
	int i, j, k;
	for (k = 0; k < a.c; ++k) {
		for (j = 0; j < a.h; ++j) {
			for (i = 0; i < a.w; ++i) {
				set_pixel(both, i, j, k, get_pixel(a, i, j, k));
			}
		}
	}
	for (k = 0; k < b.c; ++k) {
		for (j = 0; j < b.h; ++j) {
			for (i = 0; i < b.w; ++i) {
				set_pixel(both, i + a.w, j, k, get_pixel(b, i, j, k));
			}
		}
	}
	return both;
}

// Draws lines between matching pixels in two images.
// image a, b: two images that have matches.
// match *matches: array of matches between a and b.
// int n: number of matches.
// int inliers: number of inliers at beginning of matches, drawn in green.
// returns: image with matches drawn between a and b on same canvas.
image draw_matches(image a, image b, match *matches, int n, int inliers)
{
	image both = both_images(a, b);
	int i, j;
	for (i = 0; i < n; ++i) {
		int bx = (int)matches[i].p.x;
		int ex = (int)matches[i].q.x;
		int by = (int)matches[i].p.y;
		int ey = (int)matches[i].q.y;
		for (j = bx; j < ex + a.w; ++j) {
			int r = (int)((float)(j - bx) / (ex + a.w - bx) * (ey - by) + by);
			set_pixel(both, j, r, 0, i<inliers ? 0.f : 1.f);
			set_pixel(both, j, r, 1, i<inliers ? 1.f : 0.f);
			set_pixel(both, j, r, 2, 0);
		}
	}
	return both;
}

// Draw the matches with inliers in green between two images.
// image a, b: two images to match.
// matches *
image draw_inliers(image a, image b, matrix H, match *m, int n, float thresh)
{
	int inliers = model_inliers(H, m, n, thresh);
	image lines = draw_matches(a, b, m, n, inliers);
	return lines;
}

// Find corners, match them, and draw them between two images.
// image a, b: images to match.
// float sigma: gaussian for harris corner detector. Typical: 2
// float thresh: threshold for corner/no corner. Typical: 1-5
// int nms: window to perform nms on. Typical: 3
image find_and_draw_matches(image a, image b, float sigma, float thresh, int nms)
{
	int an = 0;
	int bn = 0;
	int mn = 0;
	descriptor *ad = harris_corner_detector(a, sigma, thresh, nms, &an);
	descriptor *bd = harris_corner_detector(b, sigma, thresh, nms, &bn);
	match *m = match_descriptors(ad, an, bd, bn, &mn);

	mark_corners(a, ad, an);
	mark_corners(b, bd, bn);
	image lines = draw_matches(a, b, m, mn, 0);

	free_descriptors(ad, an);
	free_descriptors(bd, bn);
	free(m);
	return lines;
}

// Calculates L1 distance between to floating point arrays.
// float *a, *b: arrays to compare.
// int n: number of values in each array.
// returns: l1 distance between arrays (sum of absolute differences).
float l1_distance(float *a, float *b, int n)
{
	float sum = 0.0;
	for (int i = 0; i < n; i++) {
		sum += fabsf(*(a + i) - *(b + i));
	}
	return sum;
}

void throw_out(match* arr, int index, int size) {
	for (int i = index; i < size - 1; i++) {
		*(arr + i) = *(arr + i + 1);
	}
}

int array_contains(int* seen, int current, int n) {
	for (int i = 0; i < n; i++) {
		if (*(seen + i) == current) {
			return 1;
		}
	}
	return 0;
}

// Finds best matches between descriptors of two images.
// descriptor *a, *b: array of descriptors for pixels in two images.
// int an, bn: number of descriptors in arrays a and b.
// int *mn: pointer to number of matches found, to be filled in by function.
// returns: best matches found. each descriptor in a should match with at most
//          one other descriptor in b.
match *match_descriptors(descriptor *a, int an, descriptor *b, int bn, int *mn)
{

	// We will have at most an matches.
	*mn = an;
	match *m = calloc(an, sizeof(match));
	for (int j = 0; j < an; j++) {
		// TODO: for every descriptor in a, find best match in b.
		// record ai as the index in *a and bi as the index in *b.
		int bind = 0; // <- find the best match
		descriptor a_desc = *(a + j);
		float min = FLT_MAX;
		for (int i = 0; i < bn; i++) {
			descriptor b_desc = *(b + i);
			float dist = l1_distance(a_desc.data, b_desc.data, b_desc.n);
			if (dist < min) {
				min = dist;
				bind = i;
			}
		}
		m[j].ai = j;
		m[j].bi = bind; // <- should be index in b.
		m[j].p = a[j].p;
		m[j].q = b[bind].p;
		m[j].distance = min; // <- should be the smallest L1 distance!
	}
	int count = 0;
	int *seen = calloc(bn, sizeof(int));
	// TODO: we want matches to be injective (one-to-one).
	// Sort matches based on distance using match_compare and qsort.
	// Then throw out matches to the same element in b. Use seen to keep track.
	// Each point should only be a part of one match.
	// Some points will not be in a match.
	// In practice just bring good matches to front of list, set *mn.
	int throw = 0;
	qsort((void*)m, an, sizeof(match), match_compare);
	for (int k = 0; k < an; k++) {
		int current = m[k].bi;
		if (array_contains(seen, current, count) == 1) {
			throw += 1;
			throw_out(m, k, an);
			k--;
			an--;
		}
		else {
			*(seen + count) = current;
			count += 1;
		}
	}
	*mn = count;
	free(seen);
	return m;
}

// Apply a projective transformation to a point.
// matrix H: homography to project point.
// point p: point to project.
// returns: point projected using the homography.
point project_point(matrix H, point p)
{
	matrix c = make_matrix(3, 1);
	// TODO: project point p with homography H.
	// Remember that homogeneous coordinates are equivalent up to scalar.
	// Have to divide by.... something...
	c.data[0][0] = p.x;
	c.data[1][0] = p.y;
	c.data[2][0] = 1;
	matrix mult = matrix_mult_matrix(H, c);
	double x = mult.data[0][0];
	double y = mult.data[1][0];
	double w = mult.data[2][0];
	point q = make_point((float)(x / w), (float)(y / w));
	return q;
}

// Calculate L2 distance between two points.
// point p, q: points.
// returns: L2 distance between them.
float point_distance(point p, point q)
{
	float dist = powf((p.x - q.x), 2.0) + powf((p.y - q.y), 2.0);
	dist = sqrtf(dist);
	return dist;
}

// Count number of inliers in a set of matches. Should also bring inliers
// to the front of the array.
// matrix H: homography between coordinate systems.
// match *m: matches to compute inlier/outlier.
// int n: number of matches in m.
// float thresh: threshold to be an inlier.
// returns: number of inliers whose projected point falls within thresh of
//          their match in the other image. Should also rearrange matches
//          so that the inliers are first in the array. For drawing.
int model_inliers(matrix H, match *m, int n, float thresh)
{
	int count = 0;
	int back = n - 1;
	// TODO: count number of matches that are inliers
	// i.e. distance(H*p, q) < thresh
	// Also, sort the matches m so the inliers are the first 'count' elements.
	for (int i = 0; i <= back; i++) {
		match current = *(m + i);
		point proj = project_point(H, current.p);
		if (point_distance(proj, current.q) < thresh) {
			count++;
		}
		else {
			match mback = *(m + back);
			match curr = *(m + i);
			*(m + back) = curr;
			*(m + i) = mback;
			back--;
			i--;
		}
	}
	return count;
}

// Randomly shuffle matches for RANSAC.
// match *m: matches to shuffle in place.
// int n: number of elements in matches.
void randomize_matches(match *m, int n)
{
	// TODO: implement Fisher-Yates to shuffle the array.
	for (int i = n - 1; i > 0; i--) {
		int ran = rand() % (i + 1);
		match mback = *(m + i);
		*(m + i) = *(m + ran);
		*(m + ran) = mback;
	}
}

// Computes homography between two images given matching pixels.
// match *matches: matching points between images.
// int n: number of matches to use in calculating homography.
// returns: matrix representing homography H that maps image a to image b.
matrix compute_homography(match *matches, int n)
{
	matrix M = make_matrix(n * 2, 8);
	matrix b = make_matrix(n * 2, 1);

	for (int i = 0; i < n; i++) {
		double mx = matches[i].p.x;
		double nx = matches[i].q.x;
		double my = matches[i].p.y;
		double ny = matches[i].q.y;
		M.data[2 * i][0] = mx;
		M.data[2 * i][1] = my;
		M.data[2 * i][2] = 1;
		M.data[2 * i][3] = 0;
		M.data[2 * i][4] = 0;
		M.data[2 * i][5] = 0;
		M.data[2 * i][6] = -mx * nx;
		M.data[2 * i][7] = -my * nx;
		M.data[2 * i + 1][0] = 0;
		M.data[2 * i + 1][1] = 0;
		M.data[2 * i + 1][2] = 0;
		M.data[2 * i + 1][3] = mx;
		M.data[2 * i + 1][4] = my;
		M.data[2 * i + 1][5] = 1;
		M.data[2 * i + 1][6] = -mx * ny;
		M.data[2 * i + 1][7] = -my * ny;
		b.data[2 * i][0] = nx;
		b.data[2 * i + 1][0] = ny;
		// TODO: fill in the matrices M and b.
	}
	matrix a = solve_system(M, b);
	free_matrix(M); free_matrix(b);

	// If a solution can't be found, return empty matrix;
	matrix none = { 0 };
	if (!a.data) return none;

	matrix H = make_matrix(3, 3);
	// TODO: fill in the homography H based on the result in a.
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (!(i == 2 && j == 2)) {
				int index = i * 3 + j;
				H.data[i][j] = a.data[index][0];
			}
		}
	}
	H.data[2][2] = 1;
	free_matrix(a);
	return H;
}

// Perform RANdom SAmple Consensus to calculate homography for noisy matches.
// match *m: set of matches.
// int n: number of matches.
// float thresh: inlier/outlier distance threshold.
// int k: number of iterations to run.
// int cutoff: inlier cutoff to exit early.
// returns: matrix representing most common homography between matches.
matrix RANSAC(match *m, int n, float thresh, int k, int cutoff)
{
	int best = 0;
	matrix Hb = make_translation_homography(256, 0);
	// TODO: fill in RANSAC algorithm.
	// for k iterations:
	//     shuffle the matches
	//     compute a homography with a few matches (how many??)
	//     if new homography is better than old (how can you tell?):
	//         compute updated homography using all inliers
	//         remember it and how good it is
	//         if it's better than the cutoff:
	//             return it immediately
	// if we get to the end return the best homography
	for (int i = 0; i < k; i++) {
		randomize_matches(m, n);
		matrix H = compute_homography(m, 4);
		int curr_inliers = model_inliers(H, m, n, thresh);
		if (curr_inliers > best) {
			best = curr_inliers;
			for (int a = 0; a < 3; a++) {
				for (int b = 0; b < 3; b++) {
					Hb.data[a][b] = H.data[a][b];
				}
			}
			if (best > cutoff) {
				return Hb;
			}
		}
	}
	return Hb;
}

int in_bounds(point x, image im) {
	if (x.x >= 0 && x.y >= 0) {
		if (x.x <= im.w && x.y <= im.h) {
			return 1;
		}
	}
	return 0;
}

// Stitches two images together using a projective transformation.
// image a, b: images to stitch.
// matrix H: homography from image a coordinates to image b coordinates.
// returns: combined image stitched together.
image combine_images(image a, image b, matrix H)
{
	matrix Hinv = matrix_invert(H);

	// Project the corners of image b into image a coordinates.
	point c1 = project_point(Hinv, make_point(0.f, 0.f));
	point c2 = project_point(Hinv, make_point(b.w - 1.f, 0.f));
	point c3 = project_point(Hinv, make_point(0.f, b.h - 1.f));
	point c4 = project_point(Hinv, make_point(b.w - 1.f, b.h - 1.f));

	// Find top left and bottom right corners of image b warped into image a.
	point topleft, botright;
	botright.x = MAX(c1.x, MAX(c2.x, MAX(c3.x, c4.x)));
	botright.y = MAX(c1.y, MAX(c2.y, MAX(c3.y, c4.y)));
	topleft.x = MIN(c1.x, MIN(c2.x, MIN(c3.x, c4.x)));
	topleft.y = MIN(c1.y, MIN(c2.y, MIN(c3.y, c4.y)));

	// Find how big our new image should be and the offsets from image a.
	int dx = (int)MIN(0, topleft.x);
	int dy = (int)MIN(0, topleft.y);
	int w = (int)(MAX(a.w, botright.x) - dx);
	int h = (int)(MAX(a.h, botright.y) - dy);
	// Can disable this if you are making very big panoramas.
	// Usually this means there was an error in calculating H.
	/*
	if(w > 7000 || h > 7000){
	fprintf(stderr, "output too big, stopping\n");
	return copy_image(a);
	}
	*/

	int i, j, k;
	image c = make_image(w, h, a.c);

	// Paste image a into the new image offset by dx and dy.
	for (k = 0; k < a.c; ++k) {
		for (j = 0; j < a.h; ++j) {
			for (i = 0; i < a.w; ++i) {
				// TODO: fill in.
				float a_pix = get_pixel(a, i, j, k);
				set_pixel(c, i - dx, j - dy, k, a_pix);
			}
		}
	}

	// TODO: Paste in image b as well.
	// You should loop over some points in the new image (which? all?)
	// and see if their projection from a coordinates to b coordinates falls
	// inside of the bounds of image b. If so, use bilinear interpolation to
	// estimate the value of b at that projection, then fill in image c.
	for (k = 0; k < a.c; ++k) {
		for (j = 0; j < c.h; ++j) {
			for (i = 0; i < c.w; ++i) {
				point b_point = project_point(H, make_point((float)i, (float)j));
				if (in_bounds(b_point, b)) {
					float b_pix = bilinear_interpolate(b, b_point.x, b_point.y, k);
					set_pixel(c, i - dx, j - dy, k, b_pix);
				}
			}
		}
	}


	return c;
}

// Create a panoramam between two images.
// image a, b: images to stitch together.
// float sigma: gaussian for harris corner detector. Typical: 2
// float thresh: threshold for corner/no corner. Typical: 1-5
// int nms: window to perform nms on. Typical: 3
// float inlier_thresh: threshold for RANSAC inliers. Typical: 2-5
// int iters: number of RANSAC iterations. Typical: 1,000-50,000
// int cutoff: RANSAC inlier cutoff. Typical: 10-100
image panorama_image(image a, image b, float sigma, float thresh, int nms, float inlier_thresh, int iters, int cutoff)
{
	srand(10);
	int an = 0;
	int bn = 0;
	int mn = 0;

	// Calculate corners and descriptors
	descriptor *ad = harris_corner_detector(a, sigma, thresh, nms, &an);
	descriptor *bd = harris_corner_detector(b, sigma, thresh, nms, &bn);

	// Find matches
	match *m = match_descriptors(ad, an, bd, bn, &mn);

	// Run RANSAC to find the homography
	matrix H = RANSAC(m, mn, inlier_thresh, iters, cutoff);

	if (0) {
		// Mark corners and matches between images
		mark_corners(a, ad, an);
		mark_corners(b, bd, bn);
		image inlier_matches = draw_inliers(a, b, H, m, mn, inlier_thresh);
		save_image(inlier_matches, "inliers");
	}

	free_descriptors(ad, an);
	free_descriptors(bd, bn);
	free(m);

	// Stitch the images together with the homography
	image comb = combine_images(a, b, H);
	return comb;
}

// Project an image onto a cylinder.
// image im: image to project.
// float f: focal length used to take image (in pixels).
// returns: image projected onto cylinder, then flattened.
image cylindrical_project(image im, float f)
{
	float xc = im.w / 2.f;
	float yc = im.h / 2.f;
	//TODO: project image onto a cylinder
	image c = copy_image(im);
	for (int i = 0; i < im.w; i++) {
		for (int j = 0; j < im.h; j++) {
			for (int k = 0; k < im.c; k++) {
				float theta = (i - xc) / f;
				float x = sinf(theta);
				float y = (j - yc) / f;
				float z = cosf(theta);
				float xs = f * (x / z) + xc;
				float ys = f * (y / z) + yc;
				float pix = get_pixel(im, i, j, k);
				set_pixel(c, (int)xs, (int)ys, k, pix);
			}
		}
	}
	return c;
}