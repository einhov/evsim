#include "food_herbivore_widget.h"
#include "ui_food_herbivore_widget.h"

#include "herbivore_neat.h"

const int food_herbivore_widget::epoch_event::event_type = QEvent::registerEventType();

food_herbivore_widget::food_herbivore_widget(evsim::food::herbivore_neat *herbivore, QWidget *parent) :
    QWidget(parent),
	species(herbivore),
	ui(new Ui::food_herbivore_widget)
{
    ui->setupUi(this);
	herbivore->widget = this;
}

food_herbivore_widget::~food_herbivore_widget() {
	species->widget = {};
	delete ui;
}

bool food_herbivore_widget::event(QEvent *e) {
	if(e->type() == epoch_event::event_type) {
		auto ev = static_cast<epoch_event*>(e);
		ui->fitness->insert_fitness(ev->epoch, ev->average_fitness, ev->minimum_fitness, ev->maximum_fitness);
		return true;
	}
	return QWidget::event(e);
}

void food_herbivore_widget::on_vision_toggle_clicked(bool checked) {
	species->draw_vision = checked;
}

void food_herbivore_widget::on_vision_texture_activated(int index) {
	species->vision_texture = index;
}
