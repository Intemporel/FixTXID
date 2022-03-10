#include "fix.h"

void Fix::run()
{
    if ( getInformation() )
        updateTextures();
    else emit sendError(model, error_type);
}

bool Fix::getInformation()
{
    QFile file(model);

    if (!file.exists())
    {
        error_type = 0;
        return false;
    }

    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    in.skipRawData(0x4);
    in >> MD20Size;

    if (MD20Size <= 264)
    {
        error_type = 1;
        file.close();
        return false;
    }

    in.skipRawData(0x50);
    in >> nTextures;
    in >> offTextures;

    in.device()->seek(MD20Size + 0x8);

    QString magic;
    qint8 skip_error = 0;
    while (true)
    {
        QByteArray buf(0x4, 0);
        in.readRawData(buf.data(), (int)0x4);
        magic = QString::fromUtf8(buf);

        if (magic == "TXID")
        {
            in.skipRawData(0x4);
            break;
        }
        else
        {
            skip_error++;

            if ( skip_error > 10 )
            {
                error_type = 2;
                file.close();
                return false;
            }

            qint32 tempPos;
            in >> tempPos;
            in.skipRawData(tempPos);
        }
    }

    for (int i = 0; i < nTextures; ++i)
    {
        qint32 data;
        in >> data;
        txid.push_back(data);
    }

    file.close();
    return true;
}

void Fix::updateTextures()
{
    QFile file(model);

    if (MD20Size <= 264)
    {
        error_type = 1;
        return;
    }

    QVector<qint32> textureOffsets;
    QVector<bool> hardcoded;

    for (int i = 0; i < nTextures; ++i)
        texture.push_back(Dictonnary[txid[i]]);

    if (!file.exists())
    {
        error_type = 0;
        return;
    }

    file.open(QIODevice::ReadWrite);
    QDataStream in(&file);
    in.setByteOrder(QDataStream::LittleEndian);

    in.device()->seek(offTextures + 0x8);
    for (int i = 0; i < nTextures; ++i)
    {
        qint32 type;
        in >> type;
        in.skipRawData(0x8);

        qint32 temp;
        in >> temp;
        textureOffsets.push_back(temp);

        if ( type == 0 ) hardcoded.push_back(true);
        else hardcoded.push_back(false);
    }

    int totalTexturesSize = 0;
    for (int i = 0; i < nTextures; ++i)
        if ( hardcoded[i] )
            totalTexturesSize += texture[i].length() + 1;

    in.device()->seek(MD20Size);
    QByteArray movePart((in.device()->size() - MD20Size), 0);
    in.readRawData(movePart.data(), movePart.size());

    in.device()->seek(0x4);
    in << (MD20Size + totalTexturesSize);

    in.device()->seek(MD20Size);
    in << (qint8)(movePart.size());

    in.device()->seek(MD20Size + totalTexturesSize);
    in.writeRawData(movePart.data(), movePart.size());

    in.device()->seek(offTextures + 0x8);
    int j = 0;
    for (int i = 0; i < nTextures; ++i)
    {
        if ( hardcoded[i] )
        {
            if ( textureOffsets[i] == 0 )
            {
                in.skipRawData(0xC);
                if ( j == 0 )
                {
                    textureOffsets[i] = MD20Size;
                    in << textureOffsets[i];
                }
                else
                {
                    textureOffsets[i] = (textureOffsets[i - 1] + (texture[i - 1].length() + 1));
                    in << textureOffsets[i];
                }
                j++;
            }
            else
                in.skipRawData(0x10);
        }
        else
            in.skipRawData(0x10);
    }

    MD20Size += totalTexturesSize;

    in.device()->seek(offTextures + 0x8);
    for (int i = 0; i < nTextures; ++i)
    {
        if ( hardcoded[i] )
        {
            in.skipRawData(0x8);
            in << (qint32)(texture[i].length() + 1);
            in.skipRawData(0x4);
        }
        else
            in.skipRawData(0x10);
    }

    for (int i = 0; i < nTextures; ++i)
    {
        if ( hardcoded[i] )
        {
            in.device()->seek(textureOffsets[i] + 0x8);
            in.writeRawData(texture[i].toUtf8().data(), texture[i].size());
            in << (qint8)0x0;
        }
    }

    file.close();
    emit updated();
}
