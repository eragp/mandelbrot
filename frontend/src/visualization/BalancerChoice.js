import React, {
    Component
} from 'react';
import BalancerPolicy from '../misc/BalancerPolicy';
import PropTypes from 'prop-types';

export default class BalancerChoice extends Component {

    constructor(props) {
        super(props);
        this.state = {
            active: 'naive',
            items: [{
                    key: 'naive',
                    title: "Naive Balancer"
                },
                {
                    key: 'column',
                    title: "Column Balancer"
                },
                {
                    key: 'prediction',
                    title: "Prediction Balancer",
                    disabled: true
                },
            ]
        };
    }

    handleBalancerChoice(balancer) {
        this.props.balancerPolicy.setBalancer(balancer);

        this.setState((state) => {
            state.active = this.props.balancerPolicy.getBalancer();
            return state;
        });
    }

    render() {
        const itemList = [];
        for (let item of this.state.items) {
            const active = item.key == this.state.active;
            const handler = item.disabled ? () => false : () => this.handleBalancerChoice(item.key);
            itemList.push(
                <BalancerItem title={item.title} active={active} disabled={item.disabled} handleClick={handler}/>
            )
        }

        return (
            <div className="list-group balancerChoice">
        {itemList}
    </div>);
    }

}

BalancerChoice.propTypes = {
    balancerPolicy: BalancerPolicy
};

class BalancerItem extends Component {

    render() {
        let classList = "list-group-item list-group-item-action";
        if (this.props.active) {
            classList += " active";
        }
        if (this.props.disabled) {
            classList += " disabled";
        }
        return <a href="#" className={classList} onClick={this.props.handleClick} >{this.props.title}</a>;
    }

}

BalancerItem.propTypes = {
    active: PropTypes.boolean,
    disabled: PropTypes.boolean,
    handleClick: PropTypes.func,
    title: PropTypes.string
};