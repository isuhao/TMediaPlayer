
#ifndef FILE_C_DIALOG_EQUALIZER
#define FILE_C_DIALOG_EQUALIZER

#include <QDialog>
#include "ui_DialogEqualizer.h"


class CApplication;


class CDialogEqualizer : public QDialog
{
    Q_OBJECT

public:

    CDialogEqualizer(CApplication * application);
    ~CDialogEqualizer();

public slots:

    void reset(void);

protected:

    int convertGainToSliderValue(double gain) const;
    double convertSliderValueToGain(int value) const;

protected slots:

    void onSlider0Change(int value);
    void onSlider1Change(int value);
    void onSlider2Change(int value);
    void onSlider3Change(int value);
    void onSlider4Change(int value);
    void onSlider5Change(int value);
    void onSlider6Change(int value);
    void onSlider7Change(int value);
    void onSlider8Change(int value);
    void onSlider9Change(int value);

private:

    Ui::DialogEqualizer * m_uiWidget;
    CApplication * m_application;
};

#endif // FILE_C_DIALOG_EQUALIZER
