// This file is part of TON Desktop Wallet,
// a desktop application for the TON Blockchain project.
//
// For license and copyright information please follow this link:
// https://github.com/ton-blockchain/wallet/blob/master/LEGAL
//
#pragma once

#include <QtCore/QString>
#include <rpl/producer.h>
#include <rpl/map.h>

namespace tr {

struct now_t {
};

inline constexpr auto now = now_t();

struct I {
	QString operator()(const QString &value) const { return value; };
};

template <typename P>
using Result = decltype(std::declval<P>()(QString()));

struct phrase {
	template <typename P = I, typename = Result<P>>
	Result<P> operator()(now_t, P p = P()) const {
		return p(value);
	};
	template <typename P = I, typename = Result<P>>
	rpl::producer<Result<P>> operator()(P p = P()) const {
		return rpl::single(value) | rpl::map(p);
	};

	QString value;
};

} // namespace tr
