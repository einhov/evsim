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
	ui->input_step->setHidden(true);
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
		ui->input_step->setHidden(false);
	}

	return QMainWindow::event(e);
}

void gui::refresh_state() {
	ui->pause->setCheckState(evsim::state.pause ? Qt::Checked : Qt::Unchecked);
	ui->draw->setCheckState(evsim::state.draw ? Qt::Checked : Qt::Unchecked);
}

void gui::step() {
	ui->generation->setText(QString("Generation: %1").arg(evsim::state.generation, -4));
	ui->step->setText(QString("Step: %1").arg(evsim::state.step));
	ui->input_step->setText(QString::number(evsim::state.step));
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
	evsim::state.pause = checked;
}

void gui::on_draw_clicked(bool checked) {
	evsim::state.draw = checked;
}

void gui::on_previous_step_clicked(bool) {
	evsim::state.skip = true;
	evsim::state.previous_step = true;
}

void gui::on_next_step_clicked(bool) {
	evsim::state.skip = true;
}

void gui::on_input_step_returnPressed() {
	evsim::state.skip = true;
	evsim::state.next_step = { ui->input_step->text().toUInt() };
}
