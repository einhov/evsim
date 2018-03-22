#ifndef GUI_H
#define GUI_H

#include <mutex>
#include <queue>

#include <QMainWindow>
#include <QEvent>
#include <QtCharts/QLineSeries>

#include "../species.h"

namespace Ui {
class gui;
}

class gui : public QMainWindow {
    Q_OBJECT

public:
    explicit gui(QWidget *parent = 0);
    ~gui();

	struct refresh_event : public QEvent {
		refresh_event() : QEvent(static_cast<Type>(event_type)) {}
		static const inline int event_type = QEvent::registerEventType();
	};

	struct quit_event : public QEvent {
		quit_event() : QEvent(static_cast<Type>(event_type)) {}
		static const inline int event_type = QEvent::registerEventType();
	};

	struct step_event : public QEvent {
		step_event() : QEvent(static_cast<Type>(event_type)) {}
		static const inline int event_type = QEvent::registerEventType();
	};

	struct add_species_event : public QEvent {
		add_species_event(evsim::species * const species) :
			QEvent(static_cast<Type>(event_type)),
			species(species) { }
		static const inline int event_type = QEvent::registerEventType();
		evsim::species * const species;
	};

	struct no_training_mode_event : public QEvent {
		no_training_mode_event(): QEvent(static_cast<Type>(event_type)) {}
		static const inline int event_type = QEvent::registerEventType();
	};

	struct data_point {
		size_t series;
		double x, y;
	};

	void append_chart_data_point(const data_point &point);
	bool event(QEvent *e) override;

private slots:
	void on_pause_clicked(bool clicked);
	void on_draw_clicked(bool clicked);
	void on_previous_step_clicked(bool);
	void on_next_step_clicked(bool);

private:
	void refresh_state();
	void step();
	void shutdown();
	void append_species(evsim::species *species);

    Ui::gui *ui;
	QtCharts::QLineSeries *series_fitness[2];
	std::mutex point_queue_mutex;
	std::queue<data_point> point_queue;
	struct { double xmin, xmax, ymin, ymax; } series_range {
		0.0, 1.0, 0.0, 1.0
	};
};

#endif // GUI_H
