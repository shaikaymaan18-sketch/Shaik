#include "qt_progress_dialog.h"

namespace QtCommon::Frontend {

QtProgressDialog::QtProgressDialog(const QString&,
                                   const QString&,
                                   int,
                                   int,
                                   QObject* parent,
                                   Qt::WindowFlags)
    : QObject(parent)
{}

QtProgressDialog::~QtProgressDialog() {}
}
