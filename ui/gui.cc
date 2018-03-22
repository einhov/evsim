#include <mutex>
#include <queue>

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QTimer>

#include "gui.h"
#include "ui_gui.h"

#include "../evsim.h"
#include "../species.h"

gui::gui(QWidget *parent) : QMainWindow(parent), ui(new Ui::gui) {
    ui->setupUi(this);
	ui->previous_step->setHidden(true);
}

gui::~gui() {
    delete ui;
}

bool gui::event(QEvent *e) {
	const auto type = e->type();
	if(type == refresh_event::event_type) {
		refresh_state();
		return true;
	} else if(type == step_event::event_type) {
		step();
		return true;
	} else if(type == quit_event::event_type) {
		shutdown();
		return true;
	} else if(type == add_species_event::event_type) {
		const auto ev = static_cast<add_species_event*>(e);
		append_species(ev->species);
	} else if(type == no_training_mode_event::event_type) {
		ui->previous_step->setHidden(false);
	}

	return QMainWindow::event(e);
}

void gui::refresh_state() {
	std::scoped_lock<std::mutex> lock(evsim::state.mutex);
	ui->pause->setCheckState(evsim::state.pause ? Qt::Checked : Qt::Unchecked);
	ui->draw->setCheckState(evsim::state.draw ? Qt::Checked : Qt::Unchecked);
}

void gui::step() {
	std::scoped_lock<std::mutex> lock(evsim::state.mutex);
	ui->generation->setText(QString("Generation: %1").arg(evsim::state.generation, -4));
	ui->step->setText(QString("Step: %1").arg(evsim::state.step));
}

void gui::shutdown() {
	close();
}

void gui::append_species(evsim::species *species) {
	if(auto widget = species->make_species_widget(); widget != nullptr) {
		widget->setParent(this);
		ui->layout_species->addWidget(widget);
	}
}

void gui::on_pause_clicked(bool checked) {
	std::scoped_lock<std::mutex> lock(evsim::state.mutex);
	evsim::state.pause = checked;
}

void gui::on_draw_clicked(bool checked) {
	std::scoped_lock<std::mutex> lock(evsim::state.mutex);
	evsim::state.draw = checked;
}

void gui::on_previous_step_clicked(bool) {
	std::scoped_lock<std::mutex> lock(evsim::state.mutex);
	evsim::state.fast_forward = true;
	evsim::state.previous_step = true;
}

void gui::on_next_step_clicked(bool) {
	std::scoped_lock<std::mutex> lock(evsim::state.mutex);
	evsim::state.fast_forward = true;
}
