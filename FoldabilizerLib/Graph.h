#pragma once

#include "FoldabilizerLibGlobal.h"

#include "Node.h"
#include "Link.h"

struct GraphState{
	QVector<Vector3>	node_scale_factor;
	QVector<int>		active_hinge_id;
	QVector<double>		hinge_angle;
	QVector<bool>		link_is_broken;
	QVector<bool>		link_is_nailed;
};


class Graph
{
public:
    Graph();
    Graph(QString fname);
    ~Graph();
	
public:
	// Modifier
    void addNode(Node* node);
	void addLink(Link* link);
	void removeNode(QString nodeID);
	void removeLink(Link* link);

	// Accessors
	int		nbNodes();
	int		nbLinks();
	bool	isEmpty();
	Node*	getNode(int idx);
	Node*	getNode(QString id);
	Link*	getLink(QString nid1, QString nid2);
	QVector<Link*> getLinks(QString nodeID);
	Node*	getBaseNode();

	// state
	GraphState getState();
	void setState(GraphState &state);

	// File I/O
	bool loadHCC(QString fname);
	bool saveHCC(QString fname);

	// Geometry property
	bool	isDrawAABB;
	Point	bbmin, bbmax, center;
	Scalar	radius;
	void	computeAabb();
	Box		getAabbBox();
	double	getAabbVolume();
	double	getMaterialVolume();

	// Visualize
	void updateHingeScale();
	void draw();

	// Prepare data
	void makeI();
	void makeL();
	void makeT();
	void makeX();
	void makeSharp();
	void makeU();
	void makeO();
	void makeChair(double legL);

	// Restore configuration
	void resetTags();
	void restoreConfiguration();

	// Shape analysis
	void			hotAnalyze();
	QVector<Node*>	getHotNodes(bool hot = true);
	QVector<Link*>	getHotLinks(bool hot = true);
	bool			isHingable(Link* link);

	// Save as obj mesh
	void saveAsObj();

public:
    QVector<Node*> nodes;
    QVector<Link*> links;

private:
	void clear();
};
