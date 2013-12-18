#include "MeshBoolean.h"
#include "Numeric.h"
#include "MeshHelper.h"

QString MeshBoolean::workPath = "C:/Development/FOLD/MeshUtilityLib/wincork/";
QString MeshBoolean::appName = "wincork";
QString MeshBoolean::boxName = "box.off";


SurfaceMeshModel* MeshBoolean::getDifference(SurfaceMeshModel* mesh, Geom::Box cutBox)
{
	// load box
	SurfaceMeshModel* boxMesh = new SurfaceMeshModel();
	QString boxPath = workPath + boxName;
	boxMesh->read(boxPath.toStdString());

	// deform to be cut box
	Geom::Box boxBox(Vector3(0,0,0), XYZ(), Vector3(1,1,1));
	cutBox.Extent *= 1.1;
	MeshHelper::deformMeshByBoxes(boxMesh, boxBox, cutBox);

	// call cork
	SurfaceMeshModel* diff = cork(mesh, boxMesh, DIFF);

	// delete box mesh
	delete boxMesh;

	return diff;
}

SurfaceMeshModel* MeshBoolean::cork( SurfaceMeshModel* m1, SurfaceMeshModel* m2, OPERATOR op )
{
	// names
	QString in1 = workPath  + "in1.off";
	QString in2 = workPath + "in2.off";
	QString out = workPath + "out.off";

	// mesh files
	m1->write(in1.toStdString());
	m2->write(in2.toStdString());

	// run command
	QString command = workPath + appName + " ";
	switch (op)
	{
	case DIFF:
		command += "-diff";
		break;
	case UNION:
		command += "-union";
		break;
	case ISCT:
		command += "-isct";
		break;
	case XOR:
		command += "-xor";
		break;
	case RESOLVE:
		command += "-resolve";
		break;
	}
	command += " " + in1 + " " + in2 + " " + out;
	system(command.toStdString().c_str());

	// retrieval result
	SurfaceMeshModel *result = new SurfaceMeshModel();
	result->read(out.toStdString().c_str());

	return result;
}
