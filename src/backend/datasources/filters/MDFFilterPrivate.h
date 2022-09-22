#ifndef MDFFILTERPRIVATE_H
#define MDFFILTERPRIVATE_H

class MDFFilter;
class AbstractDataSource;
class QString;

class MDFFilterPrivate {
public:
    explicit MDFFilterPrivate(MDFFilter*);
    void write(const QString& fileName, AbstractDataSource*);

    const MDFFilter* q;
};

#endif // MDFFILTERPRIVATE_H
