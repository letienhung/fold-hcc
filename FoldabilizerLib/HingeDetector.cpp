#include "HingeDetector.h"
#include "Segment.h"
#include "Rectangle.h"

HingeDetector::HingeDetector()
{
}

QVector<Hinge> HingeDetector::getHinges( Box &box0, Box &box1 )
{
	QVector<Hinge> hinges;

	Frame				frame0 = box0.getFrame();
	Frame				frame1 = box1.getFrame();
	QVector<Segment>	edges0 = box0.getEdgeSegments();
	QVector<Segment>	edges1 = box1.getEdgeSegments();
	QVector<Rectangle>	faces0 = box0.getFaceRectangles();
	QVector<Rectangle>	faces1 = box1.getFaceRectangles();

	// Edge-Edge test
	QMap<int, int> eIdxMap;
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			Segment& e0_i = edges0[i];
			Segment& e1_j = edges1[j];
			
			// collinear and overlap
			if (e0_i.overlaps(e1_j))
			{
				eIdxMap[i] = j;
			}
		}
	}

	// Edge-Face test

	// Face-Face test


	return hinges;
}
