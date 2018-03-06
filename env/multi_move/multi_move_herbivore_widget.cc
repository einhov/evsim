#include "multi_move_herbivore_widget.h"
#include "ui_multi_move_herbivore_widget.h"

#include "herbivore_neat.h"

const int multi_move_herbivore_widget::epoch_event::event_type = QEvent::registerEventType();

multi_move_herbivore_widget::multi_move_herbivore_widget(evsim::multi_move::herbivore_neat *herbivore, QWidget *parent) :
    QWidget(parent),
	species(herbivore),
	ui(new Ui::multi_move_herbivore_widget)
{
    ui->setupUi(this);
	herbivore->widget = this;
}

multi_move_herbivore_widget::~multi_move_herbivore_widget() {
	species->widget = {};
    delete ui;
}

bool multi_move_herbivore_widget::event(QEvent *e) {
	if(e->type() == epoch_event::event_type) {
		auto ev = static_cast<epoch_event*>(e);
		ui->fitness->insert_fitness(ev->epoch, ev->average_fitness, ev->minimum_fitness, ev->maximum_fitness);
		return true;
	}
	return QWidget::event(e);
}

void multi_move_herbivore_widget::on_vision_toggle_clicked(bool checked) {
	species->draw_vision = checked;
}

void multi_move_herbivore_widget::on_vision_texture_activated(int index) {
	species->vision_texture = index;
}
