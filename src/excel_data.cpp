#include "excel_data.h"
#include "logic_except.h"

#include "qvariant.h"

DataGroup::operation_excel_data::operation_excel_data(const std::string & input_path, const std::string & input_name)
{
    file_path = input_path;
    name = input_name;
}

DataGroup::operation_excel_data::operation_excel_data::~operation_excel_data()
{
    work_books->dynamicCall("Close()");
    work_books = nullptr;
    excel.dynamicCall("Quit(void)");
    file_path.clear();
    cell_data.clear();
}

void DataGroup::operation_excel_data::read()
{
    // 调用office应用接口
    bool label = excel.setControl("Excel.Application");
    if(!label)
    {
        // 如果失败调用WPS应用接口
        label = excel.setControl("Excel.Application");
    }
    if(!label)
    {
        throw DataGroup::logic_except("没有找到office或者wps，无法打开excel");
    }
    // 不显示窗体
    excel.setProperty("Visible", false);
    // 不显示警告
    excel.setProperty("DisplayAlerts", false);
    // 读取excel文件
    work_books = excel.querySubObject("WorkBooks");
    work_books->dynamicCall("Open(const QString&)", QString::fromStdString(file_path));
    QVariant var;
    // 获取第一个工作sheet，默认第一个数据表为总表
    QAxObject * workbook = excel.querySubObject("ActiveWorkBook");
    QAxObject * worksheets = workbook->querySubObject("Sheets");
    QAxObject * sheet = worksheets->querySubObject("Item(QString)", QString::fromStdString(name));
    if (sheet != nullptr && ! sheet->isNull())
    {
        QAxObject *usedRange = sheet->querySubObject("UsedRange");
        if(nullptr == usedRange || usedRange->isNull())
        {
            throw DataGroup::logic_except("设置读取区域失败");
        }
        var = usedRange->dynamicCall("Value");
        delete usedRange;
    }
    QVariantList var_list = var.toList();
    std::size_t count = var_list.size();
    cell_data.resize(count);
    for (int var = 0; var < count; ++var)
    {
        QVariantList ch0 = var_list.at(var).toList();
        std::size_t count0 = ch0.size();
        cell_data.at(var).resize(count0);
        for (int var0 = 0; var0 < count0; ++var0)
        {
            cell_data.at(var).at(var0) = ch0.at(var0).toString().toStdString();
        }
    }
}

void DataGroup::operation_excel_data::write()
{
    // 将std::vector<std::vector<std::string>>转换成QList<QList<QVariant>>
    QList<QList<QVariant>> temp_write;
    for (const auto & ch : cell_data)
    {
        QList<QVariant> temp_list;
        for (const auto & ch_ : ch)
        {
            QVariant temp = QString::fromStdString(ch_);
            temp_list.append(temp);
        }
        temp_write.append(temp_list);
    }

    // 把QList<QList<QVariant> > 转为QVariant
    QVariant res;
    QVariantList vars;
    const int rows = temp_write.size();
    for(int i=0 ;i<rows ;++i)
    {
        vars.append(QVariant(temp_write[i]));
    }
    res = QVariant(vars);

    // 写入
    QAxObject * workbook = excel.querySubObject("ActiveWorkBook");
    QAxObject * worksheets = workbook->querySubObject("Sheets");
    QAxObject * sheet = worksheets->querySubObject("Item(QString)", QString::fromStdString(name));
    if (sheet != nullptr && ! sheet->isNull())
    {
        int row = cell_data.size();
        int col = cell_data.empty() || cell_data.at(0).empty() ? 0 : cell_data.at(0).size();
        QAxObject * range = sheet->querySubObject("UsedRange");
        range = range->querySubObject("Resize(int,int)", row,col);
        //设置所有单元格为文本属性
        range->setProperty("NumberFormatLocal", "@");
        range->setProperty("Value", res);
    }
    workbook->dynamicCall("Save()");
}
