#include "frontend.h"

namespace QtCommon::Frontend {
QString GetOpenFileName(const QString& title, const QString& dir, const QString& filter,
                              QString* selectedFilter, QFileDialog::Options options) {
    return QFileDialog::getOpenFileName((QWidget*)rootObject, title, dir, filter, selectedFilter,
                                        QFileDialog::Options(int(options)));
}

QString GetSaveFileName(const QString& title, const QString& dir, const QString& filter,
                              QString* selectedFilter, QFileDialog::Options options) {
    return QFileDialog::getSaveFileName((QWidget*)rootObject, title, dir, filter, selectedFilter,
                                        QFileDialog::Options(int(options)));
}

QString GetExistingDirectory(const QString& caption, const QString& dir,
                                   QFileDialog::Options options) {
    return QFileDialog::getExistingDirectory((QWidget*)rootObject, caption, dir,
                                             QFileDialog::Options(int(options)));
}
} // namespace QtCommon::Frontend
