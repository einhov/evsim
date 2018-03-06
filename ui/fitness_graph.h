#ifndef FITNESS_GRAPH_H
#define FITNESS_GRAPH_H

#include <QWidget>
#include <QEvent>
#include <QtCharts/QLineSeries>

#include "../species.h"

namespace Ui {
class fitness_graph;
}

class fitness_graph : public QWidget
{
	Q_OBJECT

public:
	explicit fitness_graph(QWidget *parent = 0);
	void insert_fitness(int epoch, double fitness, double minimum_fitness, double maximum_fitness);
	~fitness_graph();

private:
	Ui::fitness_graph *ui;
	QtCharts::QLineSeries *series_fitness;
	QtCharts::QLineSeries *series_min_fitness;
	QtCharts::QLineSeries *series_max_fitness;

	struct { double xmin, xmax, ymin, ymax; } series_range {
		0.0, 1.0, 0.0, 1.0
	};
};

#endif // FITNESS_GRAPH_H
