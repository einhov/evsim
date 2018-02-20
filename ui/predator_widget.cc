#include "predator_widget.h"
#include "ui_predator_widget.h"

#include "../species_neat.h"

const int predator_widget::epoch_event::event_type = QEvent::registerEventType();

predator_widget::predator_widget(evsim::predator_neat *species, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::predator_widget),
	species(species)
{
    ui->setupUi(this);

	series_fitness = new QtCharts::QLineSeries();
	auto chart_fitness = new QtCharts::QChart();
	chart_fitness->addSeries(series_fitness);
	chart_fitness->setTheme(QtCharts::QChart::ChartThemeDark);
	chart_fitness->createDefaultAxes();
	ui->fitness->setChart(chart_fitness);
	ui->fitness->setRenderHint(QPainter::Antialiasing);
	ui->fitness->setRubberBand(QtCharts::QChartView::VerticalRubberBand);
	species->widget = this;
}

predator_widget::~predator_widget() {
	species->widget = {};
    delete ui;
}

bool predator_widget::event(QEvent *e) {
	if(e->type() == epoch_event::event_type) {
		auto ev = static_cast<epoch_event*>(e);
		insert_fitness(ev->epoch, ev->average_fitness);
		return true;
	}

	return QWidget::event(e);
}

void predator_widget::on_vision_toggle_clicked(bool checked) {
	species->draw_vision = checked;
}

void predator_widget::insert_fitness(int epoch, double fitness) {
	series_fitness->append(epoch, fitness);
	if(epoch < series_range.xmin) series_range.xmin = epoch;
	if(fitness < series_range.ymin) series_range.ymin = fitness;
	if(epoch > series_range.xmax) series_range.xmax = epoch;
	if(fitness > series_range.ymax) series_range.ymax = fitness;
	ui->fitness->chart()->axisX()->setRange(series_range.xmin, series_range.xmax);
	ui->fitness->chart()->axisY()->setRange(series_range.ymin, series_range.ymax);
}
