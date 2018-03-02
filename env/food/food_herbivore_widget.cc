#include "food_herbivore_widget.h"
#include "ui_food_herbivore_widget.h"

#include "herbivore_neat.h"

const int food_herbivore_widget::epoch_event::event_type = QEvent::registerEventType();

food_herbivore_widget::food_herbivore_widget(evsim::food::herbivore_neat *herbivore, QWidget *parent) :
    QWidget(parent),
	species(herbivore),
	ui(new Ui::food_herbivore_widget)
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
	herbivore->widget = this;
}

food_herbivore_widget::~food_herbivore_widget() {
	species->widget = {};
    delete ui;
}

bool food_herbivore_widget::event(QEvent *e) {
	if(e->type() == epoch_event::event_type) {
		auto ev = static_cast<epoch_event*>(e);
		insert_fitness(ev->epoch, ev->average_fitness, ev->minimum_fitness, ev->maximum_fitness);
		return true;
	}

	return QWidget::event(e);
}

void food_herbivore_widget::on_vision_toggle_clicked(bool checked) {
	species->draw_vision = checked;
}

void food_herbivore_widget::on_vision_texture_activated(int index) {
	species->vision_texture = index;
}

void food_herbivore_widget::insert_fitness(int epoch, double fitness, double minimum_fitness, double maximum_fitness) {
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
