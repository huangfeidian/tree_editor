

#include <qcolordialog.h>
#include <qgraphicssceneevent.h>
#include <tree_editor/common/graph/node_graph.h>
#include <tree_editor/common/graph/tree_instance.h>
#include <tree_editor/common/dialogs/info_dialog.h>
#include <tree_editor/common/dialogs/editable_dialog.h>
#include <tree_editor/common/dialogs/line_dialog.h>

#include <iostream>

using namespace spiritsaway::tree_editor;



node_graph::node_graph(basic_node* _in_model, tree_instance* _in_manager,
	QColor _text_color) :
	m_model(_in_model),
	m_manager(_in_manager)
{
	m_outline = new box_outline(color_from_uint(_in_model->m_color ));
	//std::cout<<"node graph "<< _model->_idx<<" fill with color " << _in_model->color << std::endl;
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemIsFocusable, true);
	QGraphicsSimpleTextItem* _text = new QGraphicsSimpleTextItem(
		QString::fromStdString(m_model->display_text()));
	_text->setBrush(_text_color);
	auto text_bound = _text->boundingRect();


	auto total_width = text_bound.width();

	auto total_radius = QPointF(total_width / 2, text_bound.height() / 2);
	
	auto text_center = _text->pos() + total_radius;
	_text->setPos(-1 * text_center);
	addToGroup(_text);

	auto empty_space = QPointF(node_unit, node_unit);
	auto new_r = empty_space + total_radius;
	m_cur_bounding = QRectF(-new_r, new_r);
	m_outline->m_rect = m_cur_bounding;
	addToGroup(m_outline);
	_text->setZValue(1.0);
	m_outline->setZValue(0.0);
}
QVariant node_graph::itemChange(GraphicsItemChange change, const QVariant &value)
{
	if (change == QGraphicsItem::ItemSelectedChange)
	{
		m_manager->select_changed(this, value.toInt());
		return value;
	}
	else
	{
		return QGraphicsItemGroup::itemChange(change, value);
	}

}

QRectF node_graph::boundingRect() const
{
	return m_cur_bounding;
}
tree_layouter::tree_layouter(node_graph* in_root) :
	root(in_root)
{

}
void tree_layouter::run()
{
	//first get the y value of nodes;
	cacl_node_xy(root, 0);
	total_widths = std::vector<double>(node_widths.size() + 1, 0);
	double pre = 0;
	for (std::size_t i = 0; i < node_widths.size(); i++)
	{
		pre = pre + node_widths[i];
		total_widths[i + 1] = pre;
	}
	set_node_pos(root);


}
int tree_layouter::get_y()
{
	return ++next_y;
}
void tree_layouter::cacl_node_xy(node_graph* cur_node, int level)
{
	if (level == node_widths.size())
	{
		node_widths.push_back(-1);
	}
	auto cur_w = cur_node->get_width();
	//cur_node->_manager->parent->_logger->debug("node {} cur_w {}",
	//	cur_node->_model->_idx, cur_w);
	if (cur_w > node_widths[level])
	{
		node_widths[level] = cur_w;
	}
	cur_node->m_layout_x = level;
	if (cur_node->m_children.empty())
	{
		cur_node->m_layout_y = get_y();
		return;
	}
	for (auto one_node : cur_node->m_children)
	{
		cacl_node_xy(one_node, level + 1);
	}
	cur_node->m_layout_y = (cur_node->m_children[0]->m_layout_y + cur_node->m_children.back()->m_layout_y) * 0.5;

}
void tree_layouter::set_node_pos(node_graph* cur_node)
{

	double x = total_widths[cur_node->m_layout_x] + cur_node->m_layout_x * 4 * node_unit;
	double y = node_unit * 5 * (cur_node->m_layout_y - root->m_layout_y);
	//cur_node->_manager->parent->_logger->debug("node {} layout x:{} y:{}, pos x:{} y:{}",
	//cur_node->_model->_idx, cur_node->layout_x, cur_node->layout_y, x, y);
	auto cur_p = QPointF(x, y);
	cur_node->set_left_pos(cur_p);
	for (auto one_node : cur_node->m_children)
	{
		set_node_pos(one_node);
	}
}
double node_graph::get_width() const
{
	return boundingRect().width();
}
void node_graph::set_left_pos(const QPointF& pos)
{
	auto new_pos = pos + QPoint(get_width() / 2.0, 0);
	setPos(new_pos);
}
QPointF node_graph::left_pos() const
{
	return pos() - QPointF(get_width() / 2.0, 0);
}
QPointF node_graph::right_pos() const
{
	return pos() + QPointF(get_width() / 2.0, 0);
}
QPointF node_graph::center_pos() const
{
	return pos();
}
void node_graph::draw_cross(QColor color)
{
	auto p = QPen(color);
	auto width = get_width();
	auto height = boundingRect().height();
	auto left_upper = pos() + QPointF(width * -0.5, height * 0.5);
	auto left_down = pos() + QPointF(width * -0.5, height * -0.5);
	auto right_upper = pos() + QPointF(width * 0.5, height * 0.5);
	auto right_down = pos() + QPointF(width * 0.5, height * -0.5);
	auto l1 = m_manager->m_scene->addLine(QLineF(left_upper, right_down), p);
	auto l2 = m_manager->m_scene->addLine(QLineF(left_down, right_upper), p);
	l1->setZValue(1.0);
	l2->setZValue(1.0);
}

void node_graph::draw_right_triangle(QColor color)
{
	auto p = QPen(Qt::yellow);
	auto b = QBrush(color);
	b.setStyle(Qt::SolidPattern);
	QPointF begin_pos = right_pos();
	begin_pos += QPoint(node_unit, 0);
	QPolygonF polygon;
	auto height = boundingRect().height() / 2;
	auto width = node_unit;
	auto point_1 = QPointF(0, -1 * height);
	auto point_2 = QPointF(width, 0);
	auto point_3 = QPointF(0, height);
	polygon << point_1 << point_2 << point_3<<point_1;
	polygon.translate(begin_pos);
	auto cur_p = m_manager->m_scene->addPolygon(polygon, p, b);
	cur_p->setZValue(1.0);

}

void node_graph::draw_left_circle(QColor color)
{
	auto p = QPen(Qt::yellow);
	auto b = QBrush(color);
	b.setStyle(Qt::SolidPattern);
	auto height = boundingRect().height() / 2;
	QPointF begin_pos = left_pos() - QPointF(node_unit, 0) - QPointF(height, 0);

	QPointF new_r(height, height);
	auto cur_rect = QRectF(-new_r, new_r);
	cur_rect.translate(begin_pos);
	auto cur_p = m_manager->m_scene->addEllipse(cur_rect, p, b);
	cur_p->setZValue(1.0);
}
void node_graph::draw_bound(QColor color)
{
	auto p = QPen(color);
	p.setWidth(3);
	auto b = QBrush(color, Qt::NoBrush);
	auto width = get_width();
	auto height = boundingRect().height();
	auto left_down = pos() + QPointF(width * -0.5, height * -0.5);
	m_manager->m_scene->addRect(left_down.x(), left_down.y(), width, height, p, b);
	m_manager->m_logger->info("bound is {}, {}, {}, {}", left_down.x(), left_down.y(), width, height);
}
void node_graph::set_collapsed()
{
	m_model->m_is_collapsed = true;
	m_manager->refresh();
}
void node_graph::set_un_collapsed()
{
	m_model->m_is_collapsed = false;
	m_manager->refresh();
}
void node_graph::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	//_manager->_logger->debug("node {} mouseDoubleClickEventn with show_info {}", _model->_idx, _model->_show_widget->to_json().dump());
	if (!m_model->m_show_widget->empty())
	{
		set_editable();
		return;
	}
	if (m_model->can_collapse())
	{
		if (m_model->m_is_collapsed)
		{
			set_un_collapsed();
		}
		else
		{
			set_collapsed();
		}
		return;
	}
	
	
}
void node_graph::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
	auto node_menu = new QMenu();
	auto action_color = node_menu->addAction("Color");
	QObject::connect(action_color, &QAction::triggered, this, &node_graph::set_color);
	// action_color->triggered.connect(set_color);
	if (m_model->can_collapse())
	{
		if (m_model->m_is_collapsed)
		{
			auto action_expand = node_menu->addAction("Expand");
			QObject::connect(action_expand, &QAction::triggered, this, &node_graph::set_un_collapsed);
			//action_expand->triggered.connect(set_un_collapsed);
		}
		else
		{
			auto action_collapsed = node_menu->addAction("Collapse");
			QObject::connect(action_collapsed, &QAction::triggered, this, &node_graph::set_collapsed);
			//action_collapsed->triggered.connect(set_collapsed);
		}
	}
	auto action_comment = node_menu->addAction("Comment");
	QObject::connect(action_comment, &QAction::triggered, this, &node_graph::set_comment);

	if (!m_model->m_show_widget->empty())
	{
		auto action_content = node_menu->addAction("Content");
		QObject::connect(action_content, &QAction::triggered, this, &node_graph::set_editable);
	}
	if (m_manager->parent->is_read_only())
	{
		if (!(m_model->m_has_break_point))
		{
			auto action_add_breakpoint = node_menu->addAction("add_breakpoint");
			QObject::connect(action_add_breakpoint, &QAction::triggered, this, &node_graph::add_breakpoint);
		}
		else
		{
			auto action_remove_breakpoint = node_menu->addAction("remove_breakpoint");
			QObject::connect(action_remove_breakpoint, &QAction::triggered, this, &node_graph::remove_breakpoint);

		}
		
	}
	//node_menu->move(_manager->_view->mapFromScene(center_pos().x() + event->pos().x(), center_pos().y() + event->pos().y()));

	node_menu->move(m_manager->m_view->mapFromScene(center_pos().x() - event->pos().x(), center_pos().y() - event->pos().y()));
	node_menu->show();
}
void node_graph::set_color()
{
	QColor cur_color = QColorDialog::getColor(Qt::white, m_manager->window);
	if (cur_color.isValid())
	{
		
		std::uint32_t final_value = color_to_uint(cur_color);
		//std::cout << fmt::format("color_item set with color ({}, {}, {}, {} final {})",
		//	 r,g,b,a, final_value) << std::endl;

		m_model->m_color = final_value;
		m_outline->m_color = color_from_uint(final_value);
		m_outline->update();
		m_manager->set_dirty();
	}
}

void node_graph::set_outline_color(QColor _color)
{
	//_manager->_logger->info("node {} highlight color {}", _model->_idx, color_to_uint(_color));
	m_outline->m_color = _color;
	m_outline->update();
}
void node_graph::set_comment()
{
	auto comment_dialog = new line_dialog("change comment", m_model->m_comment, m_manager->window);
	auto new_comment = comment_dialog->run();
	m_model->m_comment = new_comment;
	m_manager->set_dirty();
	m_manager->refresh();
}
void node_graph::set_editable()
{
	if (m_manager->parent->is_read_only())
	{
		auto cur_info_dialog = new info_dialog(m_manager->window, m_model);
		cur_info_dialog->run();
		return;
	}
	auto edit_dialog = new editable_dialog(m_manager->window, m_model);
	edit_dialog->run();
	m_manager->set_dirty();
	m_manager->refresh();
	return;
}
void node_graph::add_breakpoint()
{
	m_model->m_has_break_point = true;
	m_manager->refresh();
}

void node_graph::remove_breakpoint()
{
	m_model->m_has_break_point = false;
	m_manager->refresh();
}