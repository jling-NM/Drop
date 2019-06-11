#ifndef SPININTACTION_H
#define SPININTACTION_H

#include <QWidgetAction>
#include <QSpinBox>


class SpinIntAction : public QWidgetAction
{
public:
    SpinIntAction(const QString& action_text, QObject* parent = nullptr);
    QSpinBox * spinBox();

private:
    QString text;
    QSpinBox * pSpinBox;
};

#endif // SPININTACTION_H
