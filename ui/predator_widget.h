#ifndef PREDATOR_WIDGET_H
#define PREDATOR_WIDGET_H

#include <QWidget>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>

#include "../predator_neat.h"

namespace Ui {
class predator_widget;
}

class predator_widget : public QWidget {
    Q_OBJECT

public:
    explicit predator_widget(evsim::predator_neat *species, QWidget *parent = 0);
    ~predator_widget();

	struct epoch_event : public QEvent {
		epoch_event(int epoch, double avg_fitness) :
			QEvent(static_cast<Type>(event_type)),
			epoch(epoch), average_fitness(avg_fitness) {}
		static const int event_type;
		const int epoch;
		const double average_fitness;
	};

	bool event(QEvent *e) override;

private slots:
	void on_vision_toggle_clicked(bool checked);

private:
	void insert_fitness(int epoch, double fitness);
	QtCharts::QLineSeries *series_fitness;
	evsim::predator_neat *species;
    Ui::predator_widget *ui;

	struct { double xmin, xmax, ymin, ymax; } series_range {
		0.0, 1.0, 0.0, 1.0
	};
};

#endif // PREDATOR_WIDGET_H
