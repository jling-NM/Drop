#include "spinintaction.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>


SpinIntAction::SpinIntAction(const QString& action_text, QObject* parent)
: QWidgetAction(parent)
, text(action_text)
{
    QWidget* widget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout();

    layout->addWidget(new QLabel(text));
    pSpinBox = new QSpinBox();
    pSpinBox->setRange(2,5);

    layout->addWidget(pSpinBox);
    layout->setMargin(2);
    widget->setLayout(layout);

    setDefaultWidget(widget);
}

QSpinBox *SpinIntAction::spinBox()
{
    return pSpinBox;
}
