#include "door_herbivore_widget.h"
#include "ui_door_herbivore_widget.h"

#include "herbivore_neat.h"

const int door_herbivore_widget::epoch_event::event_type = QEvent::registerEventType();

door_herbivore_widget::door_herbivore_widget(evsim::door::herbivore_neat *herbivore, size_t avg_window, QWidget *parent) :
    QWidget(parent),
	species(herbivore),
    ui(new Ui::door_herbivore_widget)
{
    ui->setupUi(this);
	ui->fitness->resize_avg_window(avg_window);
	herbivore->widget = this;
}

door_herbivore_widget::~door_herbivore_widget() {
	species->widget = {};
    delete ui;
}

bool door_herbivore_widget::event(QEvent *e) {
	if(e->type() == epoch_event::event_type) {
		auto ev = static_cast<epoch_event*>(e);
		ui->fitness->insert_fitness(ev->epoch, ev->average_fitness, ev->minimum_fitness, ev->maximum_fitness);
		return true;
	}
	return QWidget::event(e);
}

void door_herbivore_widget::on_vision_toggle_clicked(bool checked) {
	species->draw_vision = checked;
}

void door_herbivore_widget::on_vision_texture_activated(int index) {
	species->vision_texture = index;
}
