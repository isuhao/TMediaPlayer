
#include "CSpecialSpinBox.hpp"
#include <QLineEdit>


CSpecialSpinBox::CSpecialSpinBox(QWidget * parent) :
    QSpinBox       (parent),
    m_spacialValue (-1),
    m_savedMax     (-1)
{

}


void CSpecialSpinBox::setPlaceholderText(const QString& text)
{
    lineEdit()->setPlaceholderText(text);
}


/**
 * Modifie la valeur sp�ciale.
 *
 * La valeur sp�cial doit commencer par un chiffre valide diff�rent de 0, et
 * �tre sup�rieure � la valeur maximale.
 *
 * Attention : setMaximum() ne doit pas �tre appel�e apr�s cette m�thode.
 *
 * \param value Valeur sp�ciale.
 */

void CSpecialSpinBox::setSpecialValue(int value)
{
    if (value <= 0)
    {
        setWrapping(false);
        m_spacialValue = -1;
        setMaximum(m_savedMax);
        disconnect(this, SIGNAL(valueChanged(int)), this, SLOT(onValueChange(int)));
    }
    else
    {
        setWrapping(true);
        m_spacialValue = value;
        m_savedMax = maximum();

        if (m_spacialValue >= m_savedMax)
        {
            setMaximum(m_spacialValue);
        }

        setValue(m_spacialValue);
        connect(this, SIGNAL(valueChanged(int)), this, SLOT(onValueChange(int)), Qt::UniqueConnection);
    }
}


QString CSpecialSpinBox::textFromValue(int value) const
{
    return (value <= 0 || value == m_spacialValue ? QString() : QString::number(value));
}


void CSpecialSpinBox::onValueChange(int value)
{
    if (m_spacialValue >= 0)
    {
        setSpecialValue(-1);
    }
}