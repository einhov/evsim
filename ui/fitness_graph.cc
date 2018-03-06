#include "fitness_graph.h"
#include "ui_fitness_graph.h"

#include "../species.h"


fitness_graph::fitness_graph(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::fitness_graph)
{
	ui->setupUi(this);

	auto chart_fitness = new QtCharts::QChart();

	series_fitness = new QtCharts::QLineSeries();
	chart_fitness->addSeries(series_fitness);

	series_min_fitness = new QtCharts::QLineSeries();
	chart_fitness->addSeries(series_min_fitness);

	series_max_fitness = new QtCharts::QLineSeries();
	chart_fitness->addSeries(series_max_fitness);

	chart_fitness->setTheme(QtCharts::QChart::ChartThemeDark);
	chart_fitness->createDefaultAxes();
	ui->fitness->setChart(chart_fitness);
	ui->fitness->setRenderHint(QPainter::Antialiasing);
	ui->fitness->setRubberBand(QtCharts::QChartView::VerticalRubberBand);
}

fitness_graph::~fitness_graph()
{
	delete ui;
}

void fitness_graph::insert_fitness(int epoch, double fitness, double minimum_fitness, double maximum_fitness) {
	series_fitness->append(epoch, fitness);
	series_min_fitness->append(epoch, minimum_fitness);
	series_max_fitness->append(epoch, maximum_fitness);
	const auto [min, max] = std::minmax({ fitness, minimum_fitness, maximum_fitness });
	if(epoch < series_range.xmin) series_range.xmin = epoch;
	if(min < series_range.ymin) series_range.ymin = min;
	if(epoch > series_range.xmax) series_range.xmax = epoch;
	if(max > series_range.ymax) series_range.ymax = max;
	ui->fitness->chart()->axisX()->setRange(series_range.xmin, series_range.xmax);
	ui->fitness->chart()->axisY()->setRange(series_range.ymin, series_range.ymax);
}
