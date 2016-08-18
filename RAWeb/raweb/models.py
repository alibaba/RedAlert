# -*- coding: utf-8 -*-
import simplejson as json
from sqlalchemy import Column
from sqlalchemy import Integer, String, Float, Boolean, Text
from sqlalchemy.ext.declarative import declarative_base
Base = declarative_base()


class JsonizeObject():
    def toJson(self):
        attrMap = {}
        for k, v in self.__dict__.items():
            if k == '_sa_instance_state':
                continue
            elif k == 'content':
                contentMap = json.loads(v)
                attrMap.update(contentMap)
            else:
                attrMap[k] = v

        js = {self.id: attrMap}
        return js

    def toNormalJson(self):
        attrMap = {}
        for k, v in self.__dict__.items():
            if k == '_sa_instance_state':
                continue
            if k == 'group':
                attrMap["groupName"] = v
            else:
                attrMap[k] = v
        return attrMap

    def fromJson(self, item):
        contentDict = {}
        columnKeys = []
        import sqlalchemy
        mapper = sqlalchemy.inspect(self)
        for column in mapper.attrs:
            columnKeys.append(column.key)
        for k, v in item.items():
            if k.startswith("_"):
                continue
            if k in columnKeys:
                setattr(self, k, v)
            else:
                contentDict[k] = v
        if "content" not in item and "content" in columnKeys:
            self.content = json.dumps(contentDict)


class Policy(Base, JsonizeObject):
    __tablename__ = 'policy'

    id = Column(Integer, autoincrement=True, primary_key=True)
    group = Column(Text, nullable=False, name="groupName")
    metric = Column(Text, nullable=False)
    type = Column(Text, nullable=False)
    validTime = Column(Text, nullable=False)
    content = Column(Text, nullable=False)
    author = Column(Text, nullable=False)
    modifiedTime = Column(Text, nullable=False)

    def __init__(self, meta):
        self.fromJson(meta)

    def __repr__(self):
        return "Policy(%s)" % (self.toJson())


class AuxPolicy(Base, JsonizeObject):
    __tablename__ = 'aux_policy'

    id = Column(Integer, autoincrement=True, primary_key=True)
    group = Column(Text, nullable=False, name="groupName")
    metric = Column(Text, nullable=False)
    type = Column(Text, nullable=False)
    validTime = Column(Text, nullable=False)
    content = Column(Text, nullable=False)
    author = Column(Text, nullable=False, primary_key=True)
    modifiedTime = Column(Text, nullable=False)
    opType = Column(Text, nullable=False)

    def __init__(self, meta):
        self.fromJson(meta)

    def __repr__(self):
        return "AuxPolicy(%s)" % (self.toJson())


class Shield(Base, JsonizeObject):
    __tablename__ = 'shield'

    id = Column(Integer, autoincrement=True, primary_key=True)
    group = Column(Text, nullable=False, name="groupName")
    metric = Column(Text, nullable=False)
    host = Column(Text, nullable=False)
    endTime = Column(Text, nullable=False)
    author = Column(Text, nullable=False)
    modifiedTime = Column(Text, nullable=False)

    def __init__(self, meta):
        self.fromJson(meta)

    def __repr__(self):
        return "Shield(%s)" % (self.toJson())


class AuxShield(Base, JsonizeObject):
    __tablename__ = 'aux_shield'

    id = Column(Integer, autoincrement=True, primary_key=True)
    group = Column(Text, nullable=False, name="groupName")
    metric = Column(Text, nullable=False)
    host = Column(Text, nullable=False)
    endTime = Column(Text, nullable=False)
    author = Column(Text, nullable=False, primary_key=True)
    modifiedTime = Column(Text, nullable=False)
    opType = Column(Text, nullable=False)

    def __init__(self, meta):
        self.fromJson(meta)

    def __repr__(self):
        return "AuxShield(%s)" % (self.toJson())


class RedAlert(Base, JsonizeObject):
    __tablename__ = 'redAlert'

    id = Column(Integer, autoincrement=True, primary_key=True)
    address = Column(Text, nullable=False)
    service = Column(Text, nullable=False)
    weight = Column(Integer, nullable=False)
    author = Column(Text, nullable=False)
    modifiedTime = Column(Text, nullable=False)

    def __init__(self, meta):
        self.fromJson(meta)

    def __repr__(self):
        return "RedAlert(%s)" % (self.toJson())


class AuxRedAlert(Base, JsonizeObject):
    __tablename__ = 'aux_redAlert'

    id = Column(Integer, autoincrement=True, primary_key=True)
    address = Column(Text, nullable=False)
    service = Column(Text, nullable=False)
    weight = Column(Integer, nullable=False)
    author = Column(Text, nullable=False, primary_key=True)
    modifiedTime = Column(Text, nullable=False)
    opType = Column(Text, nullable=False)

    def __init__(self, meta):
        self.fromJson(meta)

    def __repr__(self):
        return "AuxRedAlert(%s)" % (self.toJson())


class Pair(Base, JsonizeObject):
    __tablename__ = 'pair'

    id = Column(Integer, autoincrement=True, primary_key=True)
    key = Column(Text, nullable=False)
    value = Column(Text, nullable=False)
    author = Column(Text, nullable=False)
    modifiedTime = Column(Text, nullable=False)

    def __init__(self, meta):
        self.fromJson(meta)

    def __repr__(self):
        return "Pair(%s)" % (self.toJson())


class AuxPair(Base, JsonizeObject):
    __tablename__ = 'aux_pair'

    id = Column(Integer, autoincrement=True, primary_key=True)
    key = Column(Text, nullable=False)
    value = Column(Text, nullable=False)
    author = Column(Text, nullable=False, primary_key=True)
    modifiedTime = Column(Text, nullable=False)
    opType = Column(Text, nullable=False)

    def __init__(self, meta):
        self.fromJson(meta)

    def __repr__(self):
        return "AuxPair(%s)" % (self.toJson())


class DataSource(Base, JsonizeObject):
    __tablename__ = 'dataSource'

    id = Column(Integer, autoincrement=True, primary_key=True)
    address = Column(Text, nullable=False)
    service = Column(Text, nullable=False)
    optionString = Column(Text, nullable=True)
    fetcherType = Column(Text, nullable=False)
    author = Column(Text, nullable=False)
    modifiedTime = Column(Text, nullable=False)

    def __init__(self, meta):
        self.fromJson(meta)

    def __repr__(self):
        return "dataSource(%s)" % (self.toJson())


class AuxDataSource(Base, JsonizeObject):
    __tablename__ = 'aux_dataSource'

    id = Column(Integer, autoincrement=True, primary_key=True)
    address = Column(Text, nullable=False)
    service = Column(Text, nullable=False)
    optionString = Column(Text, nullable=True)
    fetcherType = Column(Text, nullable=False)
    author = Column(Text, nullable=False)
    modifiedTime = Column(Text, nullable=False)
    opType = Column(Text, nullable=False)

    def __init__(self, meta):
        self.fromJson(meta)

    def __repr__(self):
        return "dataSource(%s)" % (self.toJson())
