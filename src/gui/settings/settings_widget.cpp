#include "settings/settings_widget.h"

#include <QBoxLayout>
#include <QLabel>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QStyle>

// enable this to apply all settings as they are modified
//#define SETTINGS_UPDATE_IMMEDIATELY
namespace hal
{
    SettingsWidget::SettingsWidget(const QString& key, QWidget* parent)
        : QFrame(parent), m_layout(new QVBoxLayout()), m_container(new QBoxLayout(QBoxLayout::TopToBottom)), m_top_bar(new QHBoxLayout()), m_name(new QLabel()), m_revert(new QToolButton()),
          m_default(new QToolButton()), m_highlight_color(52, 56, 57), m_key(key)
    {
        setFrameStyle(QFrame::NoFrame);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        m_layout->setContentsMargins(0, 0, 0, 0);
        m_layout->setSpacing(0);

        m_name->setObjectName("name-label");

        setLayout(m_layout);

        m_revert->setText("Undo");
        m_revert->setToolTip("Revert your last change");
        //m_revert->setMaximumWidth(20);
        m_revert->setVisible(false);
        m_revert->setEnabled(false);
        QSizePolicy sp_revert = m_revert->sizePolicy();
        sp_revert.setRetainSizeWhenHidden(true);
        m_revert->setSizePolicy(sp_revert);
        connect(m_revert, &QToolButton::clicked, this, &SettingsWidget::handle_rollback);
        m_default->setText("Default");
        m_default->setToolTip("Load the default value");
        //m_default->setMaximumWidth(20);
        m_default->setVisible(false);
        m_default->setEnabled(false);
        QSizePolicy sp_default = m_default->sizePolicy();
        sp_default.setRetainSizeWhenHidden(true);
        m_default->setSizePolicy(sp_default);
        connect(m_default, &QToolButton::clicked, this, &SettingsWidget::handle_reset);
        m_top_bar->addWidget(m_name);
        m_top_bar->addStretch();
        m_top_bar->addWidget(m_revert);
        m_top_bar->addWidget(m_default);
        m_layout->addLayout(m_top_bar);
        m_layout->addLayout(m_container);

        hide();
    }

    QColor SettingsWidget::highlight_color()
    {
        return m_highlight_color;
    }

    QString SettingsWidget::key()
    {
        return m_key;
    }

    void SettingsWidget::set_highlight_color(const QColor& color)
    {
        m_highlight_color = color;
    }

    void SettingsWidget::reset_labels()
    {
        for (QPair<QLabel*, QString>& pair : m_labels)
        {
            pair.first->setText(pair.second);
            if (pair.second.isEmpty())
            {
                pair.first->hide();
            }
            else
            {
                pair.first->show();
            }
        }
    }

    bool SettingsWidget::match_labels(const QString& string)
    {
        bool match_found = false;

        if (!string.isEmpty())
        {
            QString color        = m_highlight_color.name();
            QString opening_html = "<span style=\"background-color:" + color + "\">";
            QString closing_html = "</span>";
            int string_length    = string.length();
            int html_lenght      = opening_html.length() + string_length + closing_html.length();

            for (QPair<QLabel*, QString>& pair : m_labels)
            {
                int index = pair.second.indexOf(string, 0, Qt::CaseInsensitive);
                if (index != -1)
                {
                    match_found         = true;
                    QString highlighted = pair.second;
                    while (index != -1)
                    {
                        QString substring;
                        substring   = highlighted.mid(index, string_length);
                        highlighted = highlighted.remove(index, string_length);
                        highlighted.insert(index, opening_html + substring + closing_html);
                        index = highlighted.indexOf(string, index + html_lenght, Qt::CaseInsensitive);
                    }
                    pair.first->setText(highlighted);
                }
                else
                    pair.first->setText(pair.second);
            }
        }
        return match_found;
    }

    void SettingsWidget::trigger_setting_updated()
    {
        QVariant val = value();
        if (m_preview)
        {
            m_preview->update(val);
        }
        if (m_signals_enabled)
        {
            Q_EMIT setting_updated(this, key(), val);
        }
        #ifndef SETTINGS_UPDATE_IMMEDIATELY
        set_dirty(m_loaded_value != val);
        #endif
        m_default->setEnabled(m_default_value != val);
    }

    void SettingsWidget::handle_reset()
    {
        if (m_prepared)
        {
            load(m_default_value);
            trigger_setting_updated();
        }
    }

    void SettingsWidget::handle_rollback()
    {
        if (m_prepared)
        {
            load(m_loaded_value);
            trigger_setting_updated();
        }
    }

    void SettingsWidget::set_dirty(bool dirty)
    {
        m_dirty   = dirty;

        m_revert->setEnabled(dirty);

        QStyle* s = style();
        s->unpolish(this);
        s->polish(this);
    }

    bool SettingsWidget::dirty() const
    {
        return m_dirty;
    }

    void SettingsWidget::prepare(const QVariant& value, const QVariant& default_value)
    {
        m_signals_enabled = false;
        load(value);
        m_loaded_value    = value;
        m_default_value   = default_value;
        m_signals_enabled = true;
        m_prepared        = true;
        set_dirty(false);
        m_default->setEnabled(m_default_value != m_loaded_value);
    }

    void SettingsWidget::mark_saved()
    {
        set_dirty(false);
        m_loaded_value = value();
    }

    void SettingsWidget::set_conflicts(bool conflicts)
    {
        m_conflicts = conflicts;
        QStyle* s   = style();
        s->unpolish(this);
        s->polish(this);
    }

    bool SettingsWidget::conflicts() const
    {
        return m_conflicts;
    }

    void SettingsWidget::set_preview_widget(PreviewWidget* widget)
    {
        if (m_preview)
        {
            m_container->removeWidget(m_preview);
        }
        m_preview = widget;
        m_container->addWidget(m_preview);
        if (m_prepared)
        {
            m_preview->update(value());
        }
    }

    void SettingsWidget::set_preview_position(preview_position position)
    {
        QBoxLayout::Direction direction;
        switch (position)
        {
            case preview_position::bottom:
                direction = QBoxLayout::TopToBottom;
                break;
            case preview_position::right:
                direction = QBoxLayout::LeftToRight;
                break;
            default:
                return;
        }
        m_container->setDirection(direction);
    }

    void SettingsWidget::enterEvent(QEvent* event)
    {
        Q_UNUSED(event);
        m_revert->setVisible(true);
        m_default->setVisible(true);
    }

    void SettingsWidget::leaveEvent(QEvent* event)
    {
        Q_UNUSED(event);
        m_revert->setVisible(false);
        m_default->setVisible(false);
    }
}
