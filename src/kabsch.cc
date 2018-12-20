// These kabsch(mostly just SVD) functions taken from the Hybridization
// protocol of the Rosetta software suite, TMalign.cc

#include <cmath>

#include "kabsch.h"

#include "work_area.h"
#include "input_manager.h"
#include "jutil.h"
#include "debug_utils.h"
#include "test_consts.h"

namespace elfin {

namespace kabsch {

template <typename T>
using Matrix = std::vector<std::vector<T>>;

std::vector<double> Vector3f_to_vector(Vector3f const& pt) {
	std::vector<double> v;

	v.resize(3);
	v.at(0) = pt[0];
	v.at(1) = pt[1];
	v.at(2) = pt[2];

	return v;
}

Matrix<double> V3fList_to_vectors(V3fList const& pts) {
	Matrix<double> out;

	out.resize(pts.size());
	for (size_t i = 0; i < out.size(); i++)
		out.at(i) = Vector3f_to_vector(pts.at(i));

	return out;
}

void resample(V3fList const& ref,
              V3fList& pts) {
	size_t const N = ref.size();

	// Compute shape total lengths.
	float ref_tot_len = 0.0f;
	for (size_t i = 1; i < N; i++)
		ref_tot_len += ref.at(i).dist_to(ref.at(i - 1));

	float pts_tot_len = 0.0f;
	for (size_t i = 1; i < pts.size(); i++)
		pts_tot_len += pts.at(i).dist_to(pts.at(i - 1));

	// Upsample pts.
	V3fList resampled;

	// First and last points are the same.
	resampled.push_back(pts.at(0));

	float ref_prop = 0.0f, pts_prop = 0.0f;
	int mpi = 1;
	for (size_t i = 1; i < pts.size(); i++) {
		Vector3f const& base_fp_point = pts.at(i - 1);
		Vector3f const& next_fp_point = pts.at(i);
		float const base_fp_proportion = pts_prop;
		float const fp_segment = next_fp_point.dist_to(base_fp_point)
		                         / pts_tot_len;
		Vector3f const vec = next_fp_point - base_fp_point;

		pts_prop += fp_segment;
		while (ref_prop <= pts_prop && mpi < N) {
			float const mpSegment =
			    ref.at(mpi).dist_to(ref.at(mpi - 1))
			    / ref_tot_len;

			if (ref_prop + mpSegment > pts_prop)
				break;
			ref_prop += mpSegment;

			float const s = (ref_prop - base_fp_proportion)
			                / fp_segment;
			resampled.push_back(base_fp_point + (vec * s));

			mpi++;
		}
	}

	// Sometimes the last node is automatically added
	if (resampled.size() < N) {
		resampled.push_back(pts.back());
	}

	pts = resampled;
}

// Implemetation of Kabsch algoritm for finding the best rotation matrix
// ---------------------------------------------------------------------------
// x    - x(i,m) are coordinates of atom m in set x            (input)
// y    - y(i,m) are coordinates of atom m in set y            (input)
// n    - n is number of atom pairs                            (input)
// mode  - 0:calculate rms only                                (input)
// 1:calculate rms,u,t                                 (takes longer)
// rms   - sum of w*(ux+t-y)**2 over all atom pairs            (output)
// u    - u(i,j) is   rotation  matrix for best superposition  (output)
// t    - t(i)   is translation vector for best superposition  (output)
bool rosetta_kabsch(
    std::vector<std::vector<double>> const& x,
    std::vector<std::vector<double>> const& y,
    int const n,
    int const mode,
    double *rms,
    std::vector<double>& t,
    std::vector<std::vector<double>> & u ) {
	int i = 0;
	int j = 0;
	int m = 0;
	int m1 = 0;
	int l = 0;
	int k = 0;
	double e0 = 0.0f;
	double rms1 = 0.0f;
	double d = 0.0f;
	double h = 0.0f;
	double g = 0.0f;
	double cth = 0.0f;
	double sth = 0.0f;
	double sqrth = 0.0f;
	double p = 0.0f;
	double det = 0.0f;
	double sigma = 0.0f;
	double xc[3], yc[3];
	double a[3][3] = {0};
	double b[3][3] = {0};
	double r[3][3] = {0};
	double e[3] = {0};
	double rr[6] = {0};
	double ss[6] = {0};
	double sqrt3 = 1.73205080756888, tol = 0.01;
	int ip[] = {0, 1, 3, 1, 2, 4, 3, 4, 5};
	int ip2312[] = {1, 2, 0, 1};

	int a_failed = 0, b_failed = 0;
	double epsilon = 0.00000001;

	//initializtation
	*rms = 0;
	rms1 = 0;
	e0 = 0;
	for ( i = 0; i < 3; i++ ) {
		xc[i] = 0.0;
		yc[i] = 0.0;
		t[i] = 0.0;
		for ( j = 0; j < 3; j++ ) {
			u[i][j] = 0.0;
			r[i][j] = 0.0;
			a[i][j] = 0.0;
			if ( i == j ) {
				u[i][j] = 1.0;
				a[i][j] = 1.0;
			}
		}
	}

	if ( n < 1 ) return false;

	//compute centers for vector sets x, y
	for ( i = 0; i < n; i++ ) {
		xc[0] += x[i][0];
		xc[1] += x[i][1];
		xc[2] += x[i][2];

		yc[0] += y[i][0];
		yc[1] += y[i][1];
		yc[2] += y[i][2];
	}
	for ( i = 0; i < 3; i++ ) {
		xc[i] = xc[i] / n;
		yc[i] = yc[i] / n;
	}

	//compute e0 and matrix r
	for ( m = 0; m < n; m++ ) {
		for ( i = 0; i < 3; i++ ) {
			e0 += (x[m][i] - xc[i]) * (x[m][i] - xc[i]) + \
			      (y[m][i] - yc[i]) * (y[m][i] - yc[i]);
			d = y[m][i] - yc[i];
			for ( j = 0; j < 3; j++ ) {
				r[i][j] += d * (x[m][j] - xc[j]);
			}
		}
	}

	//compute determinat of matrix r
	det = r[0][0] * ( r[1][1] * r[2][2] - r[1][2] * r[2][1] )\
	      - r[0][1] * ( r[1][0] * r[2][2] - r[1][2] * r[2][0] )\
	      + r[0][2] * ( r[1][0] * r[2][1] - r[1][1] * r[2][0] );
	sigma = det;

	//compute tras(r)*r
	m = 0;
	for ( j = 0; j < 3; j++ ) {
		for ( i = 0; i <= j; i++ ) {
			rr[m] = r[0][i] * r[0][j] + r[1][i] * r[1][j] + r[2][i] * r[2][j];
			m++;
		}
	}

	double spur = (rr[0] + rr[2] + rr[5]) / 3.0;
	double cof = (((((rr[2] * rr[5] - rr[4] * rr[4]) + rr[0] * rr[5])\
	                - rr[3] * rr[3]) + rr[0] * rr[2]) - rr[1] * rr[1]) / 3.0;
	det = det * det;

	for ( i = 0; i < 3; i++ ) {
		e[i] = spur;
	}

	if ( spur > 0 ) {
		d = spur * spur;
		h = d - cof;
		g = (spur * cof - det) / 2.0 - spur * h;

		if ( h > 0 ) {
			sqrth = sqrt(h);
			d = h * h * h - g * g;
			if ( d < 0.0 ) d = 0.0;
			d = atan2( sqrt(d), -g ) / 3.0;
			cth = sqrth * cos(d);
			sth = sqrth * sqrt3 * sin(d);
			e[0] = (spur + cth) + cth;
			e[1] = (spur - cth) + sth;
			e[2] = (spur - cth) - sth;

			if ( mode != 0 ) {
				for ( l = 0; l < 3; l = l + 2 ) {
					d = e[l];
					ss[0] = (d - rr[2]) * (d - rr[5])  - rr[4] * rr[4];
					ss[1] = (d - rr[5]) * rr[1]      + rr[3] * rr[4];
					ss[2] = (d - rr[0]) * (d - rr[5])  - rr[3] * rr[3];
					ss[3] = (d - rr[2]) * rr[3]      + rr[1] * rr[4];
					ss[4] = (d - rr[0]) * rr[4]      + rr[1] * rr[3];
					ss[5] = (d - rr[0]) * (d - rr[2])  - rr[1] * rr[1];

					if ( fabs(ss[0]) <= epsilon ) ss[0] = 0.0;
					if ( fabs(ss[1]) <= epsilon ) ss[1] = 0.0;
					if ( fabs(ss[2]) <= epsilon ) ss[2] = 0.0;
					if ( fabs(ss[3]) <= epsilon ) ss[3] = 0.0;
					if ( fabs(ss[4]) <= epsilon ) ss[4] = 0.0;
					if ( fabs(ss[5]) <= epsilon ) ss[5] = 0.0;

					if ( fabs(ss[0]) >= fabs(ss[2]) ) {
						j = 0;
						if ( fabs(ss[0]) < fabs(ss[5]) ) {
							j = 2;
						}
					} else if ( fabs(ss[2]) >= fabs(ss[5]) ) {
						j = 1;
					} else {
						j = 2;
					}

					d = 0.0;
					j = 3 * j;
					for ( i = 0; i < 3; i++ ) {
						k = ip[i + j];
						a[i][l] = ss[k];
						d = d + ss[k] * ss[k];
					}

					if ( d > epsilon ) d = 1.0 / sqrt(d);
					else d = 0.0;
					for ( i = 0; i < 3; i++ ) {
						a[i][l] = a[i][l] * d;
					}
				} //for l

				d = a[0][0] * a[0][2] + a[1][0] * a[1][2] + a[2][0] * a[2][2];
				if ( (e[0] - e[1]) > (e[1] - e[2]) ) {
					m1 = 2; m = 0;
				} else {
					m1 = 0; m = 2;
				}
				p = 0;
				for ( i = 0; i < 3; i++ ) {
					a[i][m1] = a[i][m1] - d * a[i][m];
					p = p + a[i][m1] * a[i][m1];
				}
				if ( p <= tol ) {
					p = 1.0;
					for ( i = 0; i < 3; i++ ) {
						if ( p < fabs(a[i][m]) ) {
							continue;
						}
						p = fabs( a[i][m] );
						j = i;
					}
					k = ip2312[j];
					l = ip2312[j + 1];
					p = sqrt( a[k][m] * a[k][m] + a[l][m] * a[l][m] );
					if ( p > tol ) {
						a[j][m1] = 0.0;
						a[k][m1] = -a[l][m] / p;
						a[l][m1] =  a[k][m] / p;
					} else {
						a_failed = 1;
					}
				} else {
					p = 1.0 / sqrt(p);
					for ( i = 0; i < 3; i++ ) {
						a[i][m1] = a[i][m1] * p;
					}
				}
				if ( a_failed != 1 ) {
					a[0][1] = a[1][2] * a[2][0] - a[1][0] * a[2][2];
					a[1][1] = a[2][2] * a[0][0] - a[2][0] * a[0][2];
					a[2][1] = a[0][2] * a[1][0] - a[0][0] * a[1][2];
				}
			}//if(mode!=0)
		}//h>0

		//compute b anyway
		if ( mode != 0 && a_failed != 1 ) { //a is computed correctly
			//compute b
			for ( l = 0; l < 2; l++ ) {
				d = 0.0;
				for ( i = 0; i < 3; i++ ) {
					b[i][l] = r[i][0] * a[0][l] + r[i][1] * a[1][l] + r[i][2] * a[2][l];
					d = d + b[i][l] * b[i][l];
				}
				if ( d > epsilon ) {
					d = 1.0 / sqrt(d);
				} else {
					d = 0.0;
				}
				for ( i = 0; i < 3; i++ ) {
					b[i][l] = b[i][l] * d;
				}
			}
			d = b[0][0] * b[0][1] + b[1][0] * b[1][1] + b[2][0] * b[2][1];
			p = 0.0;

			for ( i = 0; i < 3; i++ ) {
				b[i][1] = b[i][1] - d * b[i][0];
				p += b[i][1] * b[i][1];
			}

			if ( p <= tol ) {
				p = 1.0;
				for ( i = 0; i < 3; i++ ) {
					if ( p < fabs(b[i][0]) ) {
						continue;
					}
					p = fabs( b[i][0] );
					j = i;
				}
				k = ip2312[j];
				l = ip2312[j + 1];
				p = sqrt( b[k][0] * b[k][0] + b[l][0] * b[l][0] );
				if ( p > tol ) {
					b[j][1] = 0.0;
					b[k][1] = -b[l][0] / p;
					b[l][1] =  b[k][0] / p;
				} else {
					b_failed = 1;
				}
			} else {
				p = 1.0 / sqrt(p);
				for ( i = 0; i < 3; i++ ) {
					b[i][1] = b[i][1] * p;
				}
			}
			if ( b_failed != 1 ) {
				b[0][2] = b[1][0] * b[2][1] - b[1][1] * b[2][0];
				b[1][2] = b[2][0] * b[0][1] - b[2][1] * b[0][0];
				b[2][2] = b[0][0] * b[1][1] - b[0][1] * b[1][0];
				//compute u
				for ( i = 0; i < 3; i++ ) {
					for ( j = 0; j < 3; j++ ) {
						u[i][j] = b[i][0] * a[j][0] + b[i][1] * a[j][1]\
						          + b[i][2] * a[j][2];
					}
				}
			}

			//compute t
			for ( i = 0; i < 3; i++ ) {
				t[i] = ((yc[i] - u[i][0] * xc[0]) - u[i][1] * xc[1])\
				       - u[i][2] * xc[2];
			}
		}//if(mode!=0 && a_failed!=1)
	} else {
		//compute t
		for ( i = 0; i < 3; i++ ) {
			t[i] = ((yc[i] - u[i][0] * xc[0]) - u[i][1] * xc[1]) - u[i][2] * xc[2];
		}
	}

	//compute rms
	for ( i = 0; i < 3; i++ ) {
		if ( e[i] < 0 ) e[i] = 0;
		e[i] = sqrt( e[i] );
	}
	d = e[2];
	if ( sigma < 0.0 ) {
		d = - d;
	}
	d = (d + e[1]) + e[0];
	rms1 = (e0 - d) - d;
	if ( rms1 < 0.0 ) rms1 = 0.0;

	*rms = rms1;
	return true;
}

// A Wrapper to call the wonderfully commented Rosetta version.
bool kabsch(
    V3fList const& mobile,
    V3fList const& ref,
    Matrix<double> & rot,
    Vector3f& tran,
    double& rms,
    int mode = 1) {
	Matrix<double> xx = V3fList_to_vectors(mobile);
	Matrix<double> yy = V3fList_to_vectors(ref);

	std::vector<double> tt = Vector3f_to_vector(tran);

	if (rot.size() < 3)
		rot.resize(3);
	for (auto& row : rot)
		if (row.size() < 3)
			row.resize(3);

	size_t const n = mobile.size();

	bool const ret_val = rosetta_kabsch(xx, yy, n, mode, &rms, tt, rot);
	tran = Vector3f(tt);

	return ret_val;
}

float score(V3fList const& points, WorkArea const& wa) {
	return score(points, wa.to_points());
}

float score(V3fList mobile, V3fList ref) {
	if (mobile.empty() or ref.empty())
		return INFINITY;

	if (ref.size() != mobile.size())
		resample(ref, mobile);

	if (ref.size() != mobile.size())
		return INFINITY;

	// Run Kabsch to get RMS
	Matrix<double> rot;
	Vector3f tran;
	double rms;

	bool const ret_val = kabsch(mobile, ref, rot, tran, rms, 0);

	NICE_PANIC(!ret_val, "Kabsch failed!\n");

	return rms;
}

TestStat test_basics() {
	msg("Testing kabsch basics\n");
	TestStat ts;

	// kabsch() return variables
	Matrix<double> rot;
	Vector3f tran;
	double rms;

	// Test that kabsch() call returns true
	{
		bool const ret_val =
		    kabsch(points10a, points10b, rot, tran, rms);

		ts.tests++;
		if (not ret_val) {
			ts.errors++;
			err("kabsch() returned false\n");
		}
	}

	// Test kabsch() rotation.
	{
		ts.tests++;
		for (size_t i = 0; i < rot.size(); i++) {
			auto const& row = rot.at(i);
			Vector3f row_vec(row);
			if (not row_vec.is_approx(points10ab_rot[i])) {
				ts.errors++;
				err("Rotation test failed: "
				    "row %lu does not approximate actual rotation row.\n"
				    "Expeced: %s\nGot: %s\n",
				    i,
				    points10ab_rot[i].to_string().c_str(),
				    row_vec.to_string().c_str());
				break;
			}
		}
	}

	// Test kabsch() translation.
	{
		ts.tests++;
		if (not tran.is_approx(points10ab_tran)) {
			ts.errors++;
			err("Translation test failed: "
			    "does not approximate actual translation.\n"
			    "Expected: %s\nGot: %s\n",
			    points10ab_tran.to_string().c_str(),
			    tran.to_string().c_str());
		}
	}

	return ts;
}

TestStat test_resample() {
	msg("Testing kabsch resample\n");
	TestStat ts;

	// Test upsampling a_fewer to B.size()
	{
		V3fList a_fewer(points10a);

		// Erase half of the points.
		a_fewer.erase(a_fewer.begin() + (a_fewer.size() / 2),
		              a_fewer.begin() + (a_fewer.size() / 2) + 1);
		assert(a_fewer.size() != points10a.size());

		resample(points10a, a_fewer);

		ts.tests++;
		if (a_fewer.size() != points10a.size()) {
			ts.errors++;
			err("Upsampling failed.\nSizes: a_fewer=%lu points10a=%lu\n",
			    a_fewer.size(), points10a.size());
		}
	}

	return ts;
}

TestStat test_score() {
	msg("Testing kabsch score\n");
	TestStat ts;

	// Test randomly transformed solution results in kabsch score 0.
	InputManager::load_test_input();
	auto& wa = begin(SPEC.work_areas())->second;

	// Identity (no transform) score 0.
	{
		float const kscore = score(quarter_snake_free_coordinates, *wa);
		ts.tests++;
		if (kscore > 1e-6) {
			ts.errors++;
			err("kabsch identity score test failed.\n"
			    "Expected 0\nGot %f\n", kscore);

			err("Hard coded points:\n");
			for (auto const& point : quarter_snake_free_coordinates) {
				raw_at(LOG_ERROR, "%s\n", point.to_string().c_str());
			}

			err("Input file points:\n");
			V3fList const& input_points = wa->to_points();
			for (auto const& point : input_points) {
				raw_at(LOG_ERROR, "%s\n", point.to_string().c_str());
			}
		}
	}

	// Test translation score 0.
	{
		Transform trans_tx({
			{	"rot", {
					{1, 0, 0},
					{0, 1, 0},
					{0, 0, 1}
				}
			},
			{"tran", {-7.7777, -30, 150.12918}}
		});

		V3fList points_test = quarter_snake_free_coordinates;
		for (auto& point : points_test) {
			point = trans_tx * point;
		}

		float const kscore = score(points_test, *wa);
		ts.tests++;
		if (kscore > 1e-6) {
			ts.errors++;
			err("kabsch translation score test failed.\n"
			    "Expected 0\nGot %f\n", kscore);

			err("Hard coded points:\n");
			for (auto const& point : points_test) {
				raw_at(LOG_ERROR, "%s\n", point.to_string().c_str());
			}

			err("Input file points:\n");
			V3fList const& input_points = wa->to_points();
			for (auto const& point : input_points) {
				raw_at(LOG_ERROR, "%s\n", point.to_string().c_str());
			}
		}
	}

	// Test rotation score 0.
	{
		Transform rot_tx({
			{	"rot", {
					{0.28878074884414673, -0.9471790194511414, -0.13949079811573029},
					{-0.5077904462814331, -0.27504783868789673, 0.8163931369781494},
					{-0.8116370439529419, -0.16492657363414764, -0.5603969693183899}
				}
			},
			{"tran", {0, 0, 0}}
		});

		V3fList points_test = quarter_snake_free_coordinates;
		for (auto& point : points_test) {
			point = rot_tx * point;
		}

		float const kscore = score(points_test, *wa);
		ts.tests++;
		if (kscore > 1e-6) {
			ts.errors++;
			err("kabsch rotation score test failed.\n"
			    "Expected 0\nGot %f\n", kscore);

			err("Hard coded points:\n");
			for (auto const& point : points_test) {
				raw_at(LOG_ERROR, "%s\n", point.to_string().c_str());
			}

			err("Input file points:\n");
			V3fList const& input_points = wa->to_points();
			for (auto const& point : input_points) {
				raw_at(LOG_ERROR, "%s\n", point.to_string().c_str());
			}
		}
	}

	// Random transformation score 0.
	{
		// This tx is produced by taking the matrix_world of a transformed
		// Blender object.
		Transform random_tx({
			{	"rot", {
					{0.2617338001728058, 0.08983021974563599, 0.9609506130218506},
					{0.9230813384056091, 0.26742106676101685, -0.27641811966896057},
					{-0.2818091809749603, 0.959383487701416, -0.012927504256367683}
				}
			},
			{"tran", {3.15165638923645, -5.339916229248047, 3.290015935897827}}
		});

		V3fList points_test = quarter_snake_free_coordinates;
		for (auto& point : points_test) {
			point = random_tx * point;
		}

		float const kscore = score(points_test, *wa);
		ts.tests++;
		if (kscore > 1e-6) {
			ts.errors++;
			err("kabsch random transform score test failed.\n"
			    "Expected 0\nGot %f\n", kscore);

			err("Hard coded points:\n");
			for (auto const& point : points_test) {
				raw_at(LOG_ERROR, "%s\n", point.to_string().c_str());
			}

			err("Input file points:\n");
			V3fList const& input_points = wa->to_points();
			for (auto const& point : input_points) {
				raw_at(LOG_ERROR, "%s\n", point.to_string().c_str());
			}
		}
	}

	return ts;
}

TestStat test() {
	msg("Testing kabsch\n");
	TestStat ts;

	ts += test_basics();
	ts += test_resample();
	ts += test_score();

	return ts;
}

}  /* kabsch */

}  /* elfin */