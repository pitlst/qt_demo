#include "excel_data.h"
#include "logic_except.h"

#include "qvariant.h"

DataGroup::operation_excel_data::operation_excel_data(const std::string & input_path)
{
    file_path = input_path;
}

DataGroup::operation_excel_data::operation_excel_data::~operation_excel_data()
{
    work_books->dynamicCall("Close()");
    work_books = nullptr;
    excel.dynamicCall("Quit(void)");
    file_path.clear();
    sheet_cell_1.clear();
    sheet_cell_2.clear();
    sheet_cell_3.clear();
    sheet_cell_4.clear();
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
    excel.setProperty("EnableEvents",false);
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
    // 对于没有数据的情况应当特殊处理，5*5的表格是一个随机的数字，没有特殊含义
    int row = cell_data.size();
    int col;
    if(row == 0)
        col = 0;
    else
        col = cell_data[0].size();
    // 将std::vector<std::vector<std::string>>转换成QList<QList<QVariant>>
    QList<QList<QVariant>> temp_write;
    if(row == 0)
    {
        temp_write.resize(1);
        temp_write[0].resize(1);
        row = 1;
        col = 1;
    }
    else if(row != 0 && col == 0)
    {
        temp_write.resize(row);
        for(int var = 0; var < row; var++)
        {
            temp_write[var].resize(1);
        }
        col = 1;
    }
    else
    {
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
        QAxObject * range = sheet->querySubObject("UsedRange");
        range = range->querySubObject("Resize(int,int)", row,col);
        //设置所有单元格为文本属性
        range->setProperty("NumberFormatLocal", "@");
        range->setProperty("Value", res);

    }
    qDebug() << "123";
    workbook->dynamicCall("Save()");
    qDebug() << "456";
}
