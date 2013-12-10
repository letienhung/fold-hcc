// Geometric Tools, LLC
#include "UtilityGlobal.h"

typedef double Real;

namespace Geom{

class  ConvexHull2 
{
public:
    ConvexHull2 (QVector<Vector2> &pnts);
	int getNumSimplices(){return mNumSimplices;}
	QVector<int>& getIndices(){return mIndices;}

private:
	class Edge
	{
	public:
		Edge (int v0, int v1);

		int GetSign (int i, QVector<Vector2> &pnts);
		void Insert (Edge* adj0, Edge* adj1);
		void DeleteSelf ();
		void DeleteAll ();
		void GetIndices (int& numIndices, QVector<int> &indices);

		int V[2];
		Edge* E[2];
		int Sign;
		int Time;
	};

private:
   QVector<Vector2> mVertices;		// The input points.
   int					mNumSimplices;
   QVector<int>		mIndices;

private:
   bool getExtremes( QVector<int> &mExtremes);
   bool Update (Edge*& hull, int i);
};

}