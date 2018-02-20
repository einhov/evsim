#include <mutex>
#include <queue>

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QTimer>

#include "gui.h"
#include "ui_gui.h"

#include "../evsim.h"
#include "../species.h"

const int gui::refresh_event::event_type = QEvent::registerEventType();
const int gui::step_event::event_type = QEvent::registerEventType();
const int gui::quit_event::event_type = QEvent::registerEventType();
const int gui::add_species_event::event_type = QEvent::registerEventType();

gui::gui(QWidget *parent) : QMainWindow(parent), ui(new Ui::gui) {
    ui->setupUi(this);
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
