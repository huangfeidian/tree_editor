#pragma once

#include <qmenu.h>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsItem>

#include "basic_node.h"

namespace spiritsaway::tree_editor
{

	class tree_instance;
	class box_outline : public QGraphicsItem
	{
	public:
		box_outline(const QColor& _in_color) :
			QGraphicsItem(),
			m_color(_in_color)

		{

		}
		QPainterPath shape() const
		{
			auto cur_path = QPainterPath();
			cur_path.addRect(m_rect);
			return cur_path;

		}
		QRectF boundingRect() const
		{
			return m_rect;
		}
		void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr)
		{
			auto p = QPen(Qt::black);
			if (isSelected())
			{
				p.setWidth(5);
				painter->setPen(p);
			}
			else
			{
				p.setWidth(1);
				painter->setPen(p);
			}
			painter->setBrush(m_color);
			float r = m_rect.height() / 8.0;
			painter->drawRoundedRect(m_rect, r, r);
		}
		QRectF m_rect;
		QColor m_color;
	};


	class node_graph :public QObject, public QGraphicsItemGroup
	{
		Q_OBJECT

	public:
		basic_node* m_model;
		tree_instance* m_manager;
		QGraphicsItem* m_selected_effect;
		std::vector<node_graph*> m_children;
		box_outline* m_outline;
	public:
		std::uint32_t m_layout_x;
		float m_layout_y;
		QRectF m_cur_bounding;
	public:
		node_graph(basic_node* _in_model, tree_instance* _in_manager,
			QColor _text_color);

		QVariant itemChange(GraphicsItemChange change, const QVariant &value);
		QRectF boundingRect() const;
		double get_width() const;
		void set_left_pos(const QPointF& pos);
		QPointF left_pos() const;
		QPointF center_pos() const;
		QPointF right_pos() const;
		void draw_cross(QColor color);
		void draw_right_triangle(QColor color);
		void draw_left_circle(QColor color);

		void draw_bound(QColor _color);
		void set_outline_color(QColor _color);
		void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);

		void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
	public slots:
		void set_collapsed();
		void set_un_collapsed();
		void set_color();
		void set_comment();
		void set_editable();
		void add_breakpoint();
		void remove_breakpoint();
	};

	class tree_layouter
	{
	public:
		tree_layouter(node_graph* in_root);
		std::vector<double> node_widths;
		std::vector<double> total_widths;
		node_graph* root;
		int next_y = -1;
		double y_offset;
		void run();
	private:
		void cacl_node_xy(node_graph* cur_node, int level);
		void set_node_pos(node_graph* cur_node);
		int get_y();

	};
}