#ifndef MULTI_FOOD_HERBIVORE_WIDGET_H
#define MULTI_FOOD_HERBIVORE_WIDGET_H

#include <QWidget>
#include <QEvent>
#include <QtCharts/QLineSeries>

#include "herbivore_neat.h"

namespace Ui {

class multi_food_herbivore_widget;
}

class multi_food_herbivore_widget : public QWidget {
    Q_OBJECT

public:
	explicit multi_food_herbivore_widget(evsim::multi_food::herbivore_neat *species, QWidget *parent = 0);
	~multi_food_herbivore_widget();

	struct epoch_event : public QEvent {
		epoch_event(int epoch, double avg_fitness, double max_fitness, double min_fitness) :
			QEvent(static_cast<Type>(event_type)),
			epoch(epoch), average_fitness(avg_fitness),
			minimum_fitness(min_fitness), maximum_fitness(max_fitness) {}
		static const int event_type;
		const int epoch;
		const double average_fitness;
		const double minimum_fitness;
		const double maximum_fitness;
	};

	bool event(QEvent *e) override;

private slots:
	void on_vision_toggle_clicked(bool checked);
	void on_vision_texture_activated(int index);

private:
	evsim::multi_food::herbivore_neat *species;
	Ui::multi_food_herbivore_widget *ui;
};

#endif // HERBIVORE_WIDGET_H
