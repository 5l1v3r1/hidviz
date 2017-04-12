#include "DeviceView.hh"

#include "hid/ControlWidget.hh"
#include "src/hid/CollectionWidget.hh"

#include <libhidx/hid/Control.hh>
#include <libhidx/hid/Collection.hh>
#include <libhidx/Interface.hh>

#include <QGridLayout>

namespace hidviz {

    DeviceView::DeviceView(libhidx::Interface& interface, QWidget *parent) : QWidget{parent}, m_interface{interface} {
        m_layout = new QGridLayout{this};

        const auto& rootItem = m_interface.getHidReportDesc();

        for(unsigned i = 0; i < rootItem->childCount(); ++i){
            auto child = rootItem->child(i);
            addItem(child, nullptr);
        }

        connect(this, &DeviceView::dataRead, this, &DeviceView::updateData);
        interface.setReadingListener([this]{emit dataRead();});
        interface.beginReading();

        auto w = new QWidget{};
        w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
        m_layout->addWidget(w, m_layout->rowCount(), 0);
    }

    void DeviceView::addItem(libhidx::hid::Item *item, hid::ItemWidget *parent) {

        hid::ItemWidget* itemWidget = nullptr;

        auto control = dynamic_cast<libhidx::hid::Control*>(item);
        auto collection = dynamic_cast<libhidx::hid::Collection*>(item);

        if (control) {
            auto controlWidget = new hid::ControlWidget{control};

            connect(controlWidget, &hid::ControlWidget::dataUpdated, this, &DeviceView::sendData);
            itemWidget = controlWidget;
        } else if(collection) {
            itemWidget = new hid::CollectionWidget{collection};
        }

        if(parent){
            parent->appendWidget(itemWidget);
        } else {
            m_layout->addWidget(itemWidget);
        }

        if(itemWidget){
            m_items.emplace_back(itemWidget);
        }

        for (unsigned i = 0; i < item->childCount(); ++i) {
            auto child = item->child(i);
            addItem(child, itemWidget);
        }
    }

    void DeviceView::updateData() {
        for(auto item: m_items){
            auto control = dynamic_cast<hid::ControlWidget*>(item);
            if(control) {
                control->updateData();
            }
        }
    }

    void DeviceView::sendData() {
        m_interface.sendData();
    }

    DeviceView::~DeviceView() {
        m_interface.setReadingListener(nullptr);
        m_interface.stopReading();
    }

}