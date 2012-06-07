
#ifndef FILE_C_SPECIAL_SPIN_BOX
#define FILE_C_SPECIAL_SPIN_BOX

#include <QSpinBox>


/**
 * Spin box permettant d'afficher un placeholder pour une valeur sp�ciale.
 * Les valeurs valides sont sup�rieures � 0. La valeur nulle est une valeur sp�ciale.
 */

class CSpecialSpinBox : public QSpinBox
{
    Q_OBJECT

public:

    explicit CSpecialSpinBox(QWidget * parent = NULL);
    void setPlaceholderText(const QString& text);
    void setSpecialValue(int value);

protected:

    virtual QString textFromValue(int value) const;

protected slots:

    void onValueChange(int value);

private:

    int m_spacialValue;
    int m_savedMax;
};

#endif // FILE_C_SPECIAL_SPIN_BOX
