#ifndef FITNESS_GRAPH_H
#define FITNESS_GRAPH_H

#include <QWidget>
#include <QEvent>
#include <QtCharts/QLineSeries>
#include <boost/circular_buffer.hpp>

#include "../species.h"

namespace Ui {
class fitness_graph;
}

class fitness_graph : public QWidget
{
	Q_OBJECT

public:
	explicit fitness_graph(QWidget *parent = 0);
	void resize_avg_window(size_t size);
	void insert_fitness(int epoch, double fitness, double minimum_fitness, double maximum_fitness);
	~fitness_graph();

private slots:
	void on_auto_range_clicked(bool checked);

private:
	bool auto_range;
	unsigned int score_count;
	unsigned int score_window_max;
	double score_total_avg;
	double score_total_min;
	double score_total_max;

	struct score_window_elem {
		double avg, min, max;
	};
	boost::circular_buffer<score_window_elem> score_window;

	Ui::fitness_graph *ui;
	QtCharts::QLineSeries *series_fitness;
	QtCharts::QLineSeries *series_min_fitness;
	QtCharts::QLineSeries *series_max_fitness;
	QtCharts::QLineSeries *series_fitness_avg;
	QtCharts::QLineSeries *series_min_fitness_avg;
	QtCharts::QLineSeries *series_max_fitness_avg;

	void append_avg(double epoch, double fitness, double minimum_fitness, double maximum_fitness);

	struct { double xmin, xmax, ymin, ymax; } series_range {
		0.0, 1.0, 0.0, 1.0
	};
};

#endif // FITNESS_GRAPH_H
