SRCHOME=/item/ups/src
subsystem:
	@echo	"@@@@@@@@开始编译统一支付平台V1.0@@@@@@@@@@"
	@echo	"******************************************"
	@echo	"*************开始编译base模块*************"
	@cd	$(SRCHOME)/base	&& make
	@sleep 1
	@echo	"*************开始编译hash模块*************"
	@cd	$(SRCHOME)/hash	&& make
	@sleep 1
	@echo	"*************开始编译unpack模块*************"
	@cd	$(SRCHOME)/unpack	&& make
	@sleep 1
	@echo	"*************开始编译commmsg模块*************"
	@cd	$(SRCHOME)/commmsg	&& make
	@sleep 1
	@echo	"*************开始编译shm模块*************"
	@cd	$(SRCHOME)/shm	&& make
	@sleep 1
	@echo	"*************开始编译server模块*************"
	@cd	$(SRCHOME)/server	&& make
	@sleep 1
	@echo	"*************开始编译monitor模块*************"
	@cd	$(SRCHOME)/monitor	&& make
	@sleep 1
	@echo	"*************开始编译cfg模块*************"
	@cd	$(SRCHOME)/cfg	&& make
	@sleep 1
	@echo	"*************开始编译channel模块*************"
	@cd	$(SRCHOME)/channel	&& make
	@sleep 1
	@echo	"@@@@@@@@@@@编译统一支付平台V1.0结束@@@@@@@@"
