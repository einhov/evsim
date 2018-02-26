#ifndef MULTI_MOVE_HERBIVORE_WIDGET_H
#define MULTI_MOVE_HERBIVORE_WIDGET_H

#include <QWidget>
#include <QEvent>
#include <QtCharts/QLineSeries>

#include "herbivore_neat.h"

namespace Ui {


class multi_move_herbivore_widget;
}

class multi_move_herbivore_widget : public QWidget {
    Q_OBJECT

public:
	explicit multi_move_herbivore_widget(evsim::multi_move::herbivore_neat *species, QWidget *parent = 0);
	~multi_move_herbivore_widget();

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
	void on_vision_texture_activated(int index);

private:
	void insert_fitness(int epoch, double fitness);
	QtCharts::QLineSeries *series_fitness;
	evsim::multi_move::herbivore_neat *species;
	Ui::multi_move_herbivore_widget *ui;

	struct { double xmin, xmax, ymin, ymax; } series_range {
		0.0, 1.0, 0.0, 1.0
	};
};

#endif // HERBIVORE_WIDGET_H
