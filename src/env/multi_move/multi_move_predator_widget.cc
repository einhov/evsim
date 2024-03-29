#include "multi_move_predator_widget.h"
#include "ui_multi_move_predator_widget.h"

#include "predator_neat.h"

const int multi_move_predator_widget::epoch_event::event_type = QEvent::registerEventType();

multi_move_predator_widget::multi_move_predator_widget(evsim::multi_move::predator_neat *species, size_t avg_window, QWidget *parent) :
    QWidget(parent),
	species(species),
	ui(new Ui::multi_move_predator_widget)
{
    ui->setupUi(this);
	ui->fitness->resize_avg_window(avg_window);
	species->widget = this;
}

multi_move_predator_widget::~multi_move_predator_widget() {
	species->widget = {};
    delete ui;
}

bool multi_move_predator_widget::event(QEvent *e) {
	if(e->type() == epoch_event::event_type) {
		auto ev = static_cast<epoch_event*>(e);
		ui->fitness->insert_fitness(ev->epoch, ev->average_fitness, ev->minimum_fitness, ev->maximum_fitness);
		return true;
	}
	return QWidget::event(e);
}

void multi_move_predator_widget::on_vision_toggle_clicked(bool checked) {
	species->draw_vision = checked;
}

void multi_move_predator_widget::on_vision_texture_activated(int index) {
	species->vision_texture = index;
}
