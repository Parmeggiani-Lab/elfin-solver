#include <cmath>

#include "math_utils.h"
#include "jutil.h"

namespace elfin
{

int _test_math_utils()
{
	int fail_count = 0;

	msg("Testing MathUtils\n");

	// Translate 1
	Point3f a(1.0f, 2.0f, 3.0f);
	Vector3f t(9.0f, 9.0f, 9.0f);
	msg("Translate point %s using %s = ",
	    a.to_string().c_str(),
	    t.to_string().c_str());

	a += t;
	raw("%s\n", a.to_string().c_str());
	if (!a.approximates(Point3f(10.0f, 11.0f, 12.0f)))
	{
		fail_count++;
		err("Translation test 1 failed\n");
	}

	// Translate 2
	t = Vector3f(-3.0f, 100.0f, 493.1337f);
	msg("Translate point %s using %s = ",
	    a.to_string().c_str(),
	    t.to_string().c_str());

	a += t;
	raw("%s\n", a.to_string().c_str());
	if (!a.approximates(Point3f(7.0f, 111.0f, 505.1337f)))
	{
		fail_count++;
		err("Translation test 2 failed\n");
	}

	// Rotate 1
	Vector3f rot_rows1[3] = {
		Vector3f(1.0f, 0.0f, 0.0f),
		Vector3f(0.0f, 1.0f, 0.0f),
		Vector3f(0.0f, 0.0f, 1.0f)
	};
	Mat3x3 r = Mat3x3(rot_rows1);
	msg("Rotate point %s using %s = ",
	    a.to_string().c_str(),
	    r.to_string().c_str());

	a = a.dot(r);
	raw("%s\n", a.to_string().c_str());
	if (!a.approximates(Point3f(7.0f, 111.0f, 505.1337f)))
	{
		fail_count++;
		err("Rotation test 1 failed\n");
	}

	// Rotate 2
	Vector3f rot_rows2[3] = {
		Vector3f(0.4f, 0.5f, 0.0f),
		Vector3f(0.5f, 1.0f, 0.0f),
		Vector3f(0.0f, 0.0f, 1.0f)
	};
	r = Mat3x3(rot_rows2);
	msg("Rotate point %s using %s = ",
	    a.to_string().c_str(),
	    r.to_string().c_str());

	a = a.dot(r);
	raw("%s\n", a.to_string().c_str());
	if (!a.approximates(Point3f(58.3f, 114.5f, 505.1337f)))
	{
		fail_count++;
		err("Rotation test 1 failed\n");
	}

	// Rotate + translate
	t = Vector3f(-9.32f, 1.001f, -0.1337f);
	Vector3f rot_rows3[3] = {
		Vector3f(0.4f, 0.1f, 0.3f),
		Vector3f(0.5f, 0.1f, 0.53f),
		Vector3f(0.9f, 0.0f, 0.01f)
	};
	r = Mat3x3(rot_rows3);
	msg("Rotate point %s using %s\nand then translate with %s = \n",
	    a.to_string().c_str(),
	    r.to_string().c_str(),
	    t.to_string().c_str());

	a = a.dot(r) + t;
	raw("%s\n", a.to_string().c_str());
	if (!a.approximates(Point3f(5.258703160630904e2, 0.182810002279120e2, 0.830926340542118e2)))
	{
		fail_count++;
		err("Rotation + translation test failed\n");
	}

	// Test pre-dot
	msg("Pre-dot rotmat %s with vector %s\n",
	    r.to_string().c_str(),
	    a.to_string().c_str());
	a = r.dot(a);
	raw("%s\n", a.to_string().c_str());
	if (!a.approximates(Point3f(237.104014947446f, 308.802344762272f, 474.114184096774f)))
	{
		fail_count++;
		err("Pre-dot test failed\n");
	}

	// Test rotmat self dot
	msg("Self-dot on rotmat %s\n",
	    r.to_string().c_str());
	Mat3x3 rdr = r.dot(r);
	raw("%s\n", rdr.to_string().c_str());
	if (!rdr.rows[0].approximates(Point3f(0.48000000912, 0.0500000015, 0.1760000045468)) ||
	        !rdr.rows[1].approximates(Point3f(0.726999965396001, 0.06000000105, 0.20830000348028)) ||
	        !rdr.rows[2].approximates(Point3f(0.3689999954404, 0.08999999897, 0.27010000356552)))
	{
		fail_count++;
		err("Self-dot test failed\n");
	}

	// Test matrix transpose
	msg("Transpose on rotmat %s\n",
	    r.to_string().c_str());
	Mat3x3 tr = r.transpose();
	raw("%s\n", tr.to_string().c_str());
	if (!tr.rows[0].approximates(Point3f(00.400000006, 0.5, 0.8999999762)) ||
	        !tr.rows[1].approximates(Point3f(0.1000000015, 0.1000000015, 0)) ||
	        !tr.rows[2].approximates(Point3f(0.3000000119, 0.5299999714, 0.009999999776)))
	{
		fail_count++;
		err("Transpose test failed\n");
	}

	if (fail_count > 0)
	{
		err("Some tests failed - total %d\n", fail_count);
	}
	else
	{
		msg("All tests passed!\n");
	}
	return 0;
}
} // namespace elfin