#ifndef CETPROGRESSINTERFACE_H
#define CETPROGRESSINTERFACE_H

class CetProgressInterface
{
public:
    virtual ~CetProgressInterface() { }
};

Q_DECLARE_INTERFACE(CetProgressInterface, "org.qter.plugin.CetProgressInterface")

#endif // CETPROGRESSINTERFACE_H